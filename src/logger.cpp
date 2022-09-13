#include "logger.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class FileLogger : public SubLogger {
    std::ostream& file;
    bool firststep;
public:
    FileLogger(std::ostream& file, const std::array<Player, 2>& players, const std::array<Team, 2>& teams): file(file) {
        firststep = true;
        json js = { {"players", players}, {"teams", teams} };
        file << "{\"start\":" <<  js << ",\"steps\":[";
    }
    ~FileLogger() override {
        file << "]}";
    }
    void addStep(const Step* step) override {
        json j = *step;
        if(not firststep) file << ',';
        else firststep = false;
        file << j << '\n';
    }
};

Logger* Logger::liveServer(){
    return this; //TODO
}

Logger* Logger::logToFile(std::ostream& stream){
    subLoggers.push_back(std::make_unique<FileLogger>(stream, players, teams));
    return this;
}

void Logger::addStep(const uptr<Step>& step) {
    for(const auto& sub : subLoggers)
        sub->addStep(step.get());
}

/*

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

    virtual json all() {
        return { 
            {"teams", self.players},
            {"steps", self.steps}
        };
    }
*/
