from serialize import Serializable
import copy

class Character(Serializable):
    uid = ord('a')

    def __init__(self, teampos : int, name : str, maxHP : int, softAtk : int, hardAtk : int, mov : int, rng : int, netWorth : int, flavor : str = ''):
        self.name = name
        self.teampos = teampos
        self.team = None
        self.maxHP = maxHP
        self.HP = maxHP
        self.tempHP = maxHP
        self.softAtk = softAtk
        self.hardAtk = hardAtk
        self.mov = mov
        self.rng = rng
        self.position = [None, None]
        self.netWorth = netWorth
        self.flavor = flavor
        self.cid = chr(Character.uid)
        Character.uid += 1
    def copy(self):
        return copy.copy(self)
    def takeDmg(self, atkType, power):
        self.HP -= power
        return power
    def buff(self):
        self.tempHP = self.tempHP + self.maxHP
        self.HP += 50
        return (self.HP, self.tempHP)

    def serialize(self):
        return {
            "name" : self.name,
            "cid" : self.cid,
            "maxHP" : self.maxHP,
            "HP" : self.HP,
            "tempHP": self.tempHP,
            "softAtk" : self.softAtk,
            "hardAtk" : self.hardAtk,
            "mov" : self.mov,
            "rng" : self.rng,
            "netWorth" : self.netWorth,
            "flavor" : self.flavor
        }
