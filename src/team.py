from character import Character
from serialize import Serializable

class Team(Serializable):
	def __init__(self, name : str, characters):
		self.name = name
		self.characters = characters
	def serialize(self):
		return { 'name' : self.name, 'characters' : self.characters }
