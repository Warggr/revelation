import asyncio
from threading import Thread, Lock
from Queue import Queue
from websockets.server import serve
import json as JSONlib
from serialize import WSJSONencoder

from player import Agent
from team import Team
from state import State, Step

class Logger:
	def __init__(self, teams : tuple[Team, Team]):
		self.teams = teams
		self.steps : list[Step] = []
	def addStep(self, step : Step):
		self.steps.append(step)
	def all(self):
		return { "teams" : self.teams, "steps" : self.steps }
	def close(self):
		pass

class LiveServerAndLogger(Logger):
	def __init__(self, teams):
		super(teams)
		self.connected = False
		self.lockConnectionStatus = Lock()
		self.messageQueue = Queue()
		self.serverThread = Thread(target=self.runServer)

	#network-thread
	def runServer(self):
		asyncio.run( websockets.serve(self.asyncServer, localhost, 8000) )
		print('Network thread exited')

	async def asyncServer(self, websocket, path):
		# on connect
		with self.lockConnectionStatus:
			self.connected = True

		await websocket.send( JSONlib.dumps(self.all(), cls=WSJSONencoder) )

		while True:
			msg = self.messageQueue.get()
			if(msg is None):
				await websocket.close()
				print('WebSocket server exited')
				return
			else:
				await websocket.send( JSONlib.dumps(step, cls=WSJSONencoder) )

	def addStep(self, step : Step):
		super().addStep(step)
		with self.lockConnectionStatus:
			if self.connected:
				self.messageQueue.put(step)

	def close(self):
		self.messageQueue.put(None)
		self.serverThread.join()

class Game:
	def __init__(self, teams : tuple[Team, Team], agents : tuple[Agent, Agent]):
		self.head = State.createStart(teams)
		self.agents = agents
	def play(self, isLiveServer : bool) -> bool:
		logger = None
		if isLiveServer:
			logger = LiveServerAndLogger()
		else:
			logger = Logger()

		while not self.head.isFinished():
			

		logger.close()
