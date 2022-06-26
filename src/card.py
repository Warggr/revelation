import random
from enum import Enum, unique

@unique
class ActionCard(Enum):
	DEFENSE = 'Defense'
	SOFTATK = 'Soft Attack'
	HARDATK = 'Hard Attack'

"""
The general purpose of seeds is to make a random number generator deterministic. We need this twice:
- to reproduce games. For this, each game contains a master seed.
- to reproduce deck shuffling during the game. For this, each deck (as part of the state object) contains its own seed
"""

NB_SEEDS_NEEDED = 3

class Deck:
	def __init__(self, cards, discards, seed):
		self.drawPile = cards
		self.discardPile = discards
		self.seed = seed
	@staticmethod
	def create(cards):
		random.shuffle(cards)
		seed = random.randint(0, 100000)
		return Deck(cards, [], seed)
	def copy(self):
		return Deck( self.drawPile.copy(), self.discardPile.copy(), self.seed )
	def discard(self, card):
		self.discardPile.append(card)
	def draw(self):
		if not self.drawPile:
			self.drawPile = self.discardPile
			self.discardPile = []
			random.seed(self.seed)
			random.shuffle(self.drawPile)
			self.seed = random.randint(0, 100000)
		return self.drawPile.pop()
	def size(self):
		return len(self.drawPile)
	def sizeconfig(self):
		return (len(self.drawPile), len(self.discardPile))
