from enum import Enum, unique
from serialize import Serializable

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

class Character(Serializable):
    uid = ord('a')

    def __init__(self, name : str, maxHP : int, softAtk : int, hardAtk : int, mov : int, rng : int, netWorth : int, flavor : str = ''):
        self.name = name
        self.maxHP = maxHP
        self.hp = maxHP
        self.softAtk = softAtk
        self.hardAtk = hardAtk
        self.mov = mov
        self.rng = rng
        self.position = [None, None]
        self.netWorth = netWorth
        self.flavor = flavor
        self.cid = chr(Character.uid)
        Character.uid += 1
    def serialize(self):
        return {
            "name" : self.name,
            "cid" : self.cid,
            "maxHP" : self.maxHP,
            "hp" : self.hp,
            "softAtk" : self.softAtk,
            "hardAtk" : self.hardAtk,
            "mov" : self.mov,
            "rng" : self.rng,
            "netWorth" : self.netWorth,
            "flavor" : self.flavor
        }
