from serialize import Serializable
import copy

class Character(Serializable):
    uid = ord('a')

    def __init__(self, teampos : int, name : str, maxHP : int, softAtk : int, hardAtk : int, mov : int, rng : int, netWorth : int, arcAtk : bool = True, flavor : str = ''):
        self.name = name
        self.teampos = teampos
        self.team = None
        self.maxHP = maxHP
        self.HP = maxHP
        self.defShieldHP = 0
        self.softAtk = softAtk
        self.hardAtk = hardAtk
        self.mov = mov
        self.rng = rng
        self.arcAtk = arcAtk
        self.lastMoved = 0
        self.position = [None, None]
        self.netWorth = netWorth
        self.flavor = flavor
        self.cid = chr(Character.uid)
        (self.turnMoved, self.turnAttacked) = (None, None)
        Character.uid += 1
    def copy(self):
        return copy.copy(self)
    def beginTurn(self):
        me = self
        if self.defShieldHP > 0:
            me = self.copy()
            me.HP += 50
            me.defShieldHP = 0
        return me
    def takeDmg(self, isHard, power):
        if self.defShieldHP > 0:
            shielded = min(self.defShieldHP, power)
            self.defShieldHP -= shielded
            power -= shielded
        self.HP -= power
        return power
    def getAtk(self, isHard, turnId):
        if self.turnAttacked == turnId - 1: # "attacked last turn"
            return 10
        elif isHard:
            return self.hardAtk
        else:
            return self.softAtk
    def buff(self):
        self.defShieldHP = self.maxHP  #min(self.maxHP, self.HP + 50)
        return self.defShieldHP

    def serialize(self):
        return {
            "name" : self.name,
            "cid" : self.cid,
            "maxHP" : self.maxHP,
            "HP" : self.HP,
            "softAtk" : self.softAtk,
            "hardAtk" : self.hardAtk,
            "mov" : self.mov,
            "rng" : self.rng,
            "netWorth" : self.netWorth,
            "flavor" : self.flavor
        }
