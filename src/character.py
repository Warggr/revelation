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
		return [ BLOOD, MERCURY, HORROR, SPECTRUM ]

class Character(Serializable):
	def __init__(self, name : str, faction : Faction, maxHP : int, atk : int, mov : int, rng : int, shi : int):
		self.name = name
		self.faction = faction
		self.maxHP = maxHP
		self.hp = maxHP
		self.atk = atk
		self.mov = mov
		self.rng = rng
		self.shi = shi
		self.position = [None, None]
	def serialize(self):
		return {
			"name" : name,
			"faction" : faction,
			"maxHP" : maxHP,
			"hp" : maxHP,
			"atk" : atk,
			"mov" : mov,
			"rng" : rng,
			"shi" : shi
		}
