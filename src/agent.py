from constants import MAX_RESOURCES, MAX_ACTIONS
from enum import Enum, unique

@unique
class ActionOrResource(Enum):
    ACTION = True
    RESOURCES = False

class MoveDecision:
    def __init__(self, frm : tuple[int, int], to : tuple[int, int]):
        self.frm = frm
        self.to = to

class Agent:
    """
    class Agent represents a decision-maker. It is an abstract class that can be implemented by an UI that interfaces with a human, or by an AI.
    """
    def __init__(self, myId):
        self.myId = myId

    def getMyPlayer(self, state : 'State') -> 'Player':
        return state.players[ self.myId ]

    def getMovement(self, state : 'State') -> MoveDecision:
        raise NotImplementedError

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

class HumanAgent(Agent):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.name = input("Hi! Please enter your name: ")
    def getMovement(self, state : 'State') -> MoveDecision:
        for i, char in enumerate(self.getMyPlayer(state).units):
            print(f'[{i}]: {char.name}')
        iSel = int(input('Enter which character to select: '))
        charSel = self.getMyPlayer(state).units[iSel]
        possibleMovs = state.allMovementsForCharacter(charSel)
        for i, mov in enumerate(possibleMovs):
            print(f'[{i}]: to ({mov[0]}, {mov[1]})')
        iSel = int(input('Enter which position to select: '))
        movSel = possibleMovs[iSel]
        return MoveDecision(charSel.position, movSel)
