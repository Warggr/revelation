import random
from enum import Enum, unique

@unique
class ActionCard(Enum):
	DEFENSE = 'Defense'
	SOFTATK = 'Soft Attack'
	HARDATK = 'Hard Attack'

class Deck:
	def __init__(self, cards, discards):
		self.drawPile = cards
		self.discardPile = discards
	@staticmethod
	def create(cards):
		random.shuffle(cards)
		return Deck(cards, [])
	def copy(self):
		return Deck( self.drawPile.copy(), self.discardPile.copy() )
	def discard(self, card):
		self.discardPile.append(card)
	def draw(self):
		if not self.drawPile:
			self.drawPile = self.discardPile
			random.shuffle(self.drawPile)
		return self.drawPile.pop()
	def size(self):
		return len(self.drawPile)
	def sizeconfig(self):
		return (len(self.drawPile), len(self.discardPile))
