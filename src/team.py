from character import Character
from serialize import Serializable

class Team(Serializable):
	def __init__(self, name : str, characters : list[Character]):
		self.name = name
		self.characters = characters
	def serialize(self):
		return { 'name' : team.name, 'characters' : self.characters }
