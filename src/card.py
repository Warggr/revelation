import random
from enum import Enum, unique
from time import time

@unique
class ActionCard(Enum):
	DEFENSE = 'Defense'
	SOFTATK = 'Soft Attack'
	HARDATK = 'Hard Attack'

class Deck:
	def __init__(self, cards, discards, seed):
		self.drawPile = cards
		self.discardPile = discards
		self.seed = seed
	@staticmethod
	def create(cards):
		#thanks to https://stackoverflow.com/a/46443707
		seed = int(time() * 256)
		random.seed(seed)
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
			random.seed(self.seed)
			random.shuffle(self.drawPile)
			self.seed = random.randint(0, 100000)
		return self.drawPile.pop()
	def size(self):
		return len(self.drawPile)
	def sizeconfig(self):
		return (len(self.drawPile), len(self.discardPile))
