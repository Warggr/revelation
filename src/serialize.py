import json
from enum import Enum
from card import ActionCard

class Serializable:
	def serialize(self):
		raise NotImplementedError

class WSJSONEncoder(json.JSONEncoder):
	def default(self, o):
		if isinstance(o, Serializable):
			return o.serialize()
		elif isinstance(o, Enum):
			return o.name
		else:
			return super().default(o)
