import asyncio
import time
from threading import Thread, Semaphore, Lock
from websockets.server import serve, WebSocketServerProtocol

"""
Convention:
all functions that are intended to be called on the network thread start with nt_
"""
class Server:
    def __init__(self):
        self.waiting_clients = {}
        self.stop_server = None
        self.loop = None
        self.serverThread = None
        self.running = False
        self.orphan_tasks = []
        self.protect_orphans = Lock()

    def __enter__(self):
        event_loop_started = Semaphore(0)
        self.serverThread = Thread(target=self.nt_start, args=(event_loop_started,))
        self.serverThread.start()
        event_loop_started.acquire() # block until the event loop has started
        return self

    def __exit__(self, type, value, traceback):
        self.close()
        self.serverThread.join()

    def nt_start(self, event_loop_started):
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        event_loop_started.release()
        self.loop.run_until_complete( self.nt_serve() )
        print('Server thread ending')

    def nt_close(self):
        self.stop_server.set_result(None)

    def close(self):
        self.loop.call_soon_threadsafe( self.nt_close )

    async def nt_serve(self):
        with self.protect_orphans:
            self.running = True
        self.stop_server = self.loop.create_future()
        async with serve(self.nt_handle_one_connection, 'localhost', 8000):
            print(f'Server running on port 8000')

            await self.stop_server

        with self.protect_orphans: # make sure that all orphan tasks are finished
            for orphan in self.orphan_tasks:
                print('Waiting for orphan...')
                await orphan
            self.running = False
        print('Corouting ending')

    def add_client(self, path, callback):
        if path in self.waiting_clients:
            raise Exception(f'Path {path} already reserved!')
        self.waiting_clients[path] = callback

    def create_task(self, coroutine):
        with self.protect_orphans:
            if not self.running:
                raise Exception('Server is not running!')
            task = self.loop.create_task(coroutine)
            self.orphan_tasks.append(task)
            return task

    async def nt_handle_one_connection(self, protocol, requested_path):
        print('Websocket handler called: ', requested_path)
        for path in self.waiting_clients:
            if path == requested_path:
                await self.waiting_clients[path](protocol)
                return
