import random
from enum import Enum, unique

@unique
class ActionCard(Enum):
	DEFENSE = 'Defense'
	SOFTATK = 'Soft Attack'
	HARDATK = 'Hard Attack'

class Deck:
	def __init__(self, cards, discards):
		self.cards = cards
		self.discard = discards
	@staticmethod
	def create(cards):
		random.shuffle(cards)
		return Deck(cards, [])
	def copy(self):
		return Deck( self.cards[:], self.discard[:] )
	def discard(card):
		self.discard.append(card)
	def draw(self):
		if not self.cards:
			self.cards = self.discard
			random.shuffle(self.cards)
		return self.cards.pop()
	def size(self):
		return len(self.cards)
	def sizeconfig(self):
		return (len(self.cards), len(self.discard))
