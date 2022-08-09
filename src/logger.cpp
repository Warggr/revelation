#include "logger.hpp"

Logger::~Logger(){
}

Logger* Logger::liveServer(){
    return this; //TODO
}

Logger* Logger::logToTerminal(){
    return this; //TODO
}

/* class BaseLogger : public Logger {
    std::array<Player, 2> players;
    std::vector<Step*> steps;
public:
    BaseLogger(std::array<Player, 2> players) : players(players) {}

    void addStep(Step* step) {
        steps.push_back(step);
    }

    def all(self)

    :
    return { "teams" : self.players, "steps" : self.steps }

    void exit() {
        now = datetime.datetime.now()
        file = open(now.strftime('%m.%d-%H:%M') + '-replay.json', 'x')
        JSONlib.
                dump(f, self
                .

                        all()

        )
    }
}

class Decorator(Logger):
    def __init__(self, parent):
        self.parent = parent
    def all(self):
        return self.parent.all()

class LiveServerAndLogger(Decorator):
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
        self.parent.addStep(step)
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
        self.parent.__exit__(type, value, traceback)

class PrintLogger(Decorator):
    def __init__(self, parent):
        super().__init__(parent)
        print('Log will be printed')
    def addStep(self, step : Step):
        self.parent.addStep(step)
        print(step)
*/
