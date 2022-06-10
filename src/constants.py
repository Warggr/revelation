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
