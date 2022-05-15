from enum import Enum, unique
from card import ActionCard, Deck

class Player:
    """
    Is part of the @class State object. It remembers the state of a player (resources, etc.) at a certain point in time.
    """
    startingAbilityDeck = [ ActionCard.DEFENSE ] * 2 +\
        [ ActionCard.HARDATK ] * 4 +\
        [ ActionCard.SOFTATK ] * 4

    def __init__(self, units, abilityDeck = Deck([])):
        self.resources = []
        self.actions = []
        self.tokens = []
        self.units = units
        self.actionDeck = Deck(Player.startingAbilityDeck)
        self.abilityDeck = abilityDeck

    def useActionCard(self, cardValue):
        self.actions.remove(cardValue)
        self.actionDeck.discard(cardValue)

    #virtual
    def getAbilityDeck(self):
        return self.abilityDeck
