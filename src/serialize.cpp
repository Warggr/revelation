import json
from constants import Faction
from card import ActionCard

class Serializable:
	def serialize(self):
		raise NotImplementedError

class WSJSONEncoder(json.JSONEncoder):
	def default(self, o):
		if isinstance(o, Serializable):
			return o.serialize()
		elif isinstance(o, Faction):
			return o.name
		elif isinstance(o, ActionCard):
			return o.name
		else:
			return super().default(o)
