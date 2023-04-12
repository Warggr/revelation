import asyncio
import pathlib
import datetime
import json as JSONlib
from threading import Thread, Lock
from queue import Queue
from websockets.server import serve

from serialize import WSJSONEncoder
from player import Player
from state import Step

class SubLogger:
    def __init__(self, parent: 'Logger'):
        self.parent = parent
    def __enter__(self):
        return self
    def __exit__(self, type, value, traceback):
        pass
    def addStep(self, step : Step):
        pass

class Logger:
    def __init__(self, state : 'State', game : 'Game'):
        self.subloggers : list[SubLogger] = []
        self.state = state
        self.game = game
        self.steps : list[Step] = []
    def addSubLogger(self, SubloggerClass, *args, **kwargs):
        self.subloggers.append( SubloggerClass(self, *args, **kwargs) )
    def addStep(self, step : Step):
        self.steps.append(step)
        for sublogger in self.subloggers:
            sublogger.addStep(step)
    def all(self):
        return { "state" : self.state.serialize(), "steps" : self.steps, "game" : self.game.serialize() }
    def __enter__(self):
        for sublogger in self.subloggers:
            sublogger.__enter__()
    def __exit__(self, type, value, traceback):
        for sublogger in self.subloggers:
            sublogger.__exit__(type, value, traceback)

class LiveServerAndLogger(SubLogger):
    def __init__(self, parent):
        super().__init__(parent)
        self.connected = False
        self.lockConnectionStatus = Lock()
        self.messageQueue = Queue()
        self.loop = asyncio.new_event_loop()
        self.serverThread = Thread(target=self.runServer)
        self.serverThread.start()

    #network-thread
    def runServer(self):
        asyncio.set_event_loop(self.loop)
        print(f'Server running! Open this URL: file://{ str(pathlib.Path(__file__).parent.parent / "viewer" / "player.html") }?liveurl=localhost%3A8000')
        self.loop.run_until_complete( serve(self.asyncServer, 'localhost', 8000) )
        self.loop.run_forever()
        print('Network thread exited')

    async def asyncServer(self, websocket, path):
        # on connect
        with self.lockConnectionStatus:
            print('Connected!')
            self.connected = True

        await websocket.send( JSONlib.dumps(self.all(), cls=WSJSONEncoder) )

        while True:
            step = self.messageQueue.get()
            if(step is None):
                await websocket.close()
                self.loop.call_soon_threadsafe(self.loop.stop)
                print('WebSocket server exited')
                return
            else:
                await websocket.send( JSONlib.dumps(step, cls=WSJSONEncoder) )

    def addStep(self, step : Step):
        with self.lockConnectionStatus:
            if self.connected:
                self.messageQueue.put(step)

    def __exit__(self, type, value, traceback):
        with self.lockConnectionStatus:
            if self.connected:
                self.messageQueue.put(None)
            else:
                self.loop.call_soon_threadsafe(self.loop.stop)

        self.serverThread.join()

class PrintLogger(SubLogger):
    def __init__(self, parent: 'Logger'):
        super().__init__(parent)
        print('Log will be printed')
    def addStep(self, step : Step):
        print(step)

class FileLogger(SubLogger):
    def __init__(self, parent: 'Logger'):
        super().__init__(parent)
    def __exit__(self, type, value, traceback):
        if type is None:
            now = datetime.datetime.now()
            file = open(now.strftime('%m.%d-%H:%M:%S') + '-replay.json', 'x')
            JSONlib.dump(self.parent.all(), file, cls=WSJSONEncoder)