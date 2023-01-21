#include "control/game.hpp"
#include "control/agent.hpp"
#include "search/depthfirstsearch.hpp"
#include "search/loggers.hpp"
#include "network/server_impl.hpp"
#include "network/network_agent.hpp"
#include "logging/file_logger.hpp"
#include "logging/network_logger.hpp"
#include "setup/agent_setup.hpp"
#include "setup/units_repository.hpp"
#include "random.hpp"
#include "nlohmann/json.hpp"
#include <boost/program_options.hpp>
#include <array>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <utility>
#include <mutex>

namespace po = boost::program_options;

using json = nlohmann::json;

void printVersion(){
    std::cout << "revelation v1\n";
}

po::variables_map readArgs(int argc, const char** argv) {
    po::variables_map vm;

    po::options_description options("Allowed options");
    options.add_options()
            ("help,h", "produce help message")
            ("version,v", "print the version number")
            ("seed,s", po::value<GeneratorSeed>()->default_value(getRandom()), "game seed")
            //("json,j", po::value<std::string>(&parsedValues.json), "game parameters in json")
            ("print-json,p", "print the allowed JSON syntax")
            ("server,s", po::value<unsigned short int>(), "Launch a live server on the given port")
            ;

    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if(vm.count("help")){
        std::cout << options << '\n';
        exit(EXIT_SUCCESS);
    }
    if(vm.count("version")){
        printVersion();
        exit(EXIT_SUCCESS);
    }
    if(vm.count("print-json")){
        std::cout << grammar_as_json_string << '\n';
        exit(EXIT_SUCCESS);
    }

    return vm;
}

/* Using a bot */
[[ maybe_unused ]]
static SearchAgent mkSearchAgent(uint myId) {
    // Decide on a heuristic (currently only PTTH is implemented)
    auto heur = std::make_unique<PowerTimesToughnessHeuristic>();
    // For depth-first search, you can optionally use a logger to log progress.
    // If you don't want logging, use NoOpLogger::getInstance().
    // auto logger = NoOpLogger::getInstance();
    auto logger = std::make_shared<ProgressBar>();
    // Use a policy. Use StaticDFS by default, all others are still experimental.
    auto policy = std::make_unique<StaticDFS>(logger, *heur);
    // You can add additional turns of planning to a StaticDFS by using setOpponentsTurn
    // policy->setOpponentsTurn(std::make_unique<StaticDFS>(logger, *heur))
    // ... possibly multiple times
    //      ->setOpponentsTurn(std::make_unique<StaticDFS>(logger, *heur));
    // Create a SearchAgent with the policy and the heuristic.
    SearchAgent searchAgent(myId, std::move(policy), std::move(heur));
    return searchAgent;
}

int main(int argc, const char* argv[]){
    auto vm = readArgs(argc, argv);

    /* Initialize teams */
    UnitsRepository repository;
    // teams.txt is assumed to be in the current directory
    std::filesystem::current_path(std::filesystem::current_path() / "resources");
    repository.mkDefaultTeams();

    /* Create bot agent */
    // <!> pass as myId the index that this will have in the list of agents, so 0 or 1!
    auto bot = mkSearchAgent(0);

    /* Disclaimer: creating a browser-based agent throws a segmentation fault for some reason. */

    /* Create (normal, browser-based) human agent */
    // For this, we need a server listening to connections
    Server_impl server("0.0.0.0", 8000);
    // Launch server in separate thread
    auto networkThread = std::thread( [&server]{ server.start(); } );
    // ...and a server room in which the game takes place
    auto [ roomId, room ] = server.addRoom();
    // Tell the room that we expect a new player/session with the given agent ID
    auto session = room.addSession(1);
    // Create NetworkAgent from the session
    auto normal_human = NetworkAgent(1, session);
    normal_human.sync_init(); // wait for the agent to come online

    /* Create a (simple, terminal-based) human agent */
    // HumanAgent simple_human(1);

    std::array<Agent*, NB_AGENTS> agents = { &bot, &normal_human };
    // You can set teams directly...
    // std::array<const Team*, NB_AGENTS> teams = { &(repository.getTeams().at("Europe")), &(repository.getTeams().at("Near East")) };
    // or ask the agents
    std::array<const Team*, NB_AGENTS> teams = { &bot.getTeam(repository), &normal_human.getTeam(repository) };

    // Set the game seed (optional)
    GeneratorSeed seed = vm["seed"].as<GeneratorSeed>();
    Game game(teams, agents, seed);
    // without seed
    // Game game(teams, agents);

    // If there's a server room, send the game events to the room
    game.logger.addSubLogger<LiveServerAndLogger>(room);

    auto results = game.play();
    std::cout << results.whoWon << "won!\n";

    // End server and corresponding thread
    server.stop();
    networkThread.join();
}
