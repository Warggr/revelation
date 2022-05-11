from player import Player
from team import Team
from character import Character, Faction
from card import Deck
from game import Game
from serialize import Serializable

class Step(Serializable):
	def __init__(self, typ, **kwargs):
		self.type = typ
		self.kwargs = kwargs
	def serialize(self):
		return { 'type' : self.type, **self.kwargs }

class State:
	@staticmethod
	def createStart(teams : tuple[Team, Team]) -> State:
		board = [ [None] * 12 for _ in range(2) ]
		for (i, team) in enumerate(teams):
			for (j, row) in enumerate(team.characters):
				for (k, char) in enumerate(row):
					board[ k + 1 + 6*i ][ j ] = char
					char.position = [ k + 1 + 6*i, j ]

		players = [ Player(game) for _ in range(2) ]

		startingResources = sum([[x] * 4 for x in Faction.allFactions()]) + [ Faction.ETHER ]
		resDeck = Deck(startingResources)

		return State(board, players, resDeck)

	def __init__(self, board : list[list[Character | NoneType]], players : list[Player], resDeck : Deck):
		self.board = board
		self.players = players
		self.resDeck = resDeck

	def copy(self):
		return State(self.board, self.players, self.resDeck)

	def getBoardField(self, coords):
		return self.board[ coords[0] ][ coords[1] ]
	def setBoardField(self, coords, value):
		self.board[ coords[0] ][ coords[1] ] = value
		if(value):
			value.position = coords

	def apply(self, step : Step) -> State:
		newState = self.copy()
		if step.type == 'pass':
			pass
		elif step.type == 'move':
			newState.board = [ row[:] for row in self.board ]
			mover = newState.getBoardField(step.kwargs['from'])
			newState.setBoardField(step.kwargs['from'], board.getBoardField(step.kwargs['to']))
			newState.setBoardField(step.kwargs['to'], mover)
		elif step.type == 'dmg':
			newState.players[ step.kwargs['who'] ].useActionCard( step.kwargs['atkType'] )
			newState.board = [ row[:] for row in self.board ]
			victim = newState.getBoardField(step.kwargs['victim'])
			victim.hp = step.kwargs['newHP']
		elif step.type == 'destroy':
			newState.board = [ row[:] for row in self.board ]
			newState.setBoardField(step.kwargs['victim'], None)
		else:
			print(step.type)
			raise NotImplementedError
		return newState

	def isFinished(self):
		return self.players[0].units.empty() or self.players[1].units.empty()
