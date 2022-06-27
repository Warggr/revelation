from enum import Enum, unique

MAX_RESOURCES = 3
MAX_ACTIONS = 4
MAX_ABILITIES = 4

@unique
class Faction(Enum):
    NONE = 0
    BLOOD = 1
    MERCURY = 2
    HORROR = 3
    SPECTRUM = 4
    ETHER = 5

    @staticmethod
    def allFactions():
        return [ Faction.BLOOD, Faction.MERCURY, Faction.HORROR, Faction.SPECTRUM ]

ARMY_WIDTH = 3
HALF_BOARD_WIDTH = ARMY_WIDTH + 2
FULL_BOARD_WIDTH = 2*HALF_BOARD_WIDTH

@unique
class Timestep(Enum):
    BEGIN = 0
    DREW = 1
    DISCARDED = 2
    MOVEDfirst = 3
    MOVEDlast = 4
    ABILITYCHOSEN = 5
    ACTED = 6

@unique
class Direction(Enum):
    UP = 0
    LEFT = 1
    RIGHT = 2
    DOWN = 3

    def inverse(self):
        return Direction( 3 - self.value )

def getNeighbour(position, direction):
    if direction is Direction.UP:
        return ( position[0] - 1, position[1] )
    elif direction is Direction.DOWN:
        return ( position[0] + 1, position[1] )
    elif direction is Direction.LEFT:
        return ( position[0], position[1] - 1 )
    elif direction is Direction.RIGHT:
        return ( position[0], position[1] + 1 )
    else:
        raise AssertionError