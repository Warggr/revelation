import asyncio
import pathlib
import datetime
import json as JSONlib
from threading import Thread, Lock
import queue
import websockets
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
    def __init__(self, parent, server):
        super().__init__(parent)
        self.connected = False
        self.server = server
        self.lockConnectionStatus = Lock()
        # The inbound queue can block this thread
        self.messageQueue_inbound = queue.Queue()
        # The outbound queue can block the network thread, so it has to be an asyncio Queue
        self.messageQueue_outbound = asyncio.Queue()

    def __enter__(self):
        self.server.add_client('/watch', self.nt_serve)
        return self

    def __exit__(self, type, value, traceback):
        print('exiting logger...')
        with self.lockConnectionStatus:
            if self.connected:
                self.messageQueue_outbound.put_nowait(None)

    async def nt_serve(self, websocket):
        with self.lockConnectionStatus:
            self.connected = True

        async def reading_coroutine(websocket):
            async for message in websocket:
                print('recv: Received message: ', message)
                self.messageQueue_inbound.put(message)

        async def sending_coroutine(websocket):
            await websocket.send( JSONlib.dumps(self.parent.all(), cls=WSJSONEncoder) )

            while True:
                step = await self.messageQueue_outbound.get()
                if(step is None):
                    return
                else:
                    await websocket.send( JSONlib.dumps(step, cls=WSJSONEncoder) )

        tasks = [ self.server.loop.create_task( coroutine(websocket) ) for coroutine in (reading_coroutine, sending_coroutine) ]
        done, pending = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
        for pending in pending:
            pending.cancel()

        with self.lockConnectionStatus:
            self.connected = False

    # asyncio.Queue is not thread-safe, so we have to modify the queue on the network thread
    def nt_addStep(self, step : Step):
        with self.lockConnectionStatus:
            if self.connected:
                self.messageQueue_outbound.put_nowait(step)

    def addStep(self, step : Step):
        self.server.loop.call_soon_threadsafe( lambda: self.nt_addStep(step) )

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
