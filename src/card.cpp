

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
