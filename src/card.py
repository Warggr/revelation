import random
from enum import Enum, unique

@unique
class ActionCard(Enum):
	DEFENSE = 1
	SOFTATK = 2
	HARDATK = 3

class Deck:
	def __init__(self, cards):
		self.cards = cards
		random.shuffle(self.cards)
		self.discard = []
	def discard(card):
		self.discard.append(card)
	def draw(self):
		if not self.cards:
			self.cards = self.discard
			random.shuffle(self.cards)
		return self.cards.pop()
