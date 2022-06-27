from enum import Enum, unique
from card import ActionCard, Deck

class Player:
    """
    Is part of the @class State object. It remembers the state of a player (resources, etc.) at a certain point in time.
    """
    startingAbilityDeck = [ ActionCard.DEFENSE ] * 2 +\
        [ ActionCard.HARDATK ] * 4 +\
        [ ActionCard.SOFTATK ] * 4

    def __init__(self, abilityDeck = Deck.create([])):
        self.resources = []
        self.actions = []
        self.tokens = []
        self.actionDeck = Deck.create(Player.startingAbilityDeck)
        self.abilityDeck = abilityDeck

    def serialize(self):
        return {
            "actionDeckSize" : self.actionDeck.sizeconfig()
        }

    def useActionCard(self, cardValue):
        self.actions.remove(cardValue)
        self.actionDeck.discard(cardValue)

    def drawAction(self):
        cardDrawn = self.actionDeck.draw()
        self.actions.append(cardDrawn)
        #print('Drew', cardDrawn, ', now have', self.actions)
        return cardDrawn

    def drawResource(self, resourceDeck):
        cardDrawn = resourceDeck.draw()
        self.resources.append(cardDrawn)
        return cardDrawn

    def discard(self, card):
        if card not in self.actions:
            print('Self.actions:', self.actions)
            print('Card', card)
        self.actions.remove(card)
        self.actionDeck.discard(card)

    #virtual
    def getAbilityDeck(self):
        return self.abilityDeck