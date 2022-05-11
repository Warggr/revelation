from enum import Enum, unique
from constants import MAX_RESOURCES, MAX_ACTIONS
from cards import ActionCard, Deck

@unique
class ActionOrResource(Enum):
	ACTION = True
	RESOURCES = False

class Agent:
	"""
	class Agent represents a decision-maker. It is an abstract class that can be implemented by an UI that interfaces with a human, or by an AI.
	"""
	def turn(self):
		self.drawAndDiscardStep()
		self.movPhase()
		self.abiPhase()
		self.actPhase()

	def drawAndDiscardStep(self):
		x : ActionOrResource = self.chooseActionOrResource()
		if x == ActionOrResource.RESOURCES:
			self.resources += self.game.resDeck.draw()
			if len(self.resources) > MAX_RESOURCES:
				self.chooseAndDiscardResource()
		else:
			self.resources += self.actDeck.draw()
			if len(self.actions) > MAX_ACTIONS:
				self.chooseAndDiscardAction()

	def movPhase(self):
		movements = self.getMovements()
		for mv in movements:
			

	def chooseActionOrResource(self) -> ActionOrResource:
		raise NotImplementedError
	def chooseAndDiscardResource(self):
		raise NotImplementedError
	def chooseAndDiscardAction(self):
		raise NotImplementedError
	def getAbilityDeck(self):
		raise NotImplementedError
	def getMovements(self):
		raise NotImplementedError

class Player:
	"""
	Is part of the @class State object. It remembers the state of a player (resources, etc.) at a certain point in time.
	"""
	startingAbilityDeck = [ ActionCard.DEFENSE ] * 2 +
		[ ActionCard.HARDATK ] * 4 +
		[ ActionCard.SOFTATK ] * 4

	def __init__(self, units):
		self.resources = []
		self.actions = []
		self.tokens = []
		self.units = []
		self.actionDeck = Deck(startingAbilityDeck)
		self.abilityDeck = self.getAbilityDeck()

	def useActionCard(self, cardValue):
		self.actions.remove(cardValue)
		self.actionDeck.discard(cardValue)
