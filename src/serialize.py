import json

class Serializable:
	def serialize(self):
		raise NotImplementedError

class WSJSONEncoder(json.JSONEncoder):
	def default(self, o):
		if isinstance(o, Serializable):
			return o.serialize()
		else:
			return super().default(o)
