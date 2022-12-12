#include "control/game.hpp"
#include "control/agent.hpp"
#include "search/depthfirstsearch.hpp"
#include "search/loggers.hpp"
#include "random.hpp"
#include "network/server.hpp"
#include "network/network_agent.hpp"
#include <boost/program_options.hpp>
#include <array>
#include <iostream>
#include <thread>
#include <utility>

namespace po = boost::program_options;

struct ProgramOptions {
    Generator::result_type seed;
};

void printVersion(){
    std::cout << "revelation v1\n";
}

std::pair<po::variables_map, ProgramOptions> readArgs(int argc, const char** argv) {
    po::variables_map rawValues;
    ProgramOptions parsedValues;

    po::options_description options("Allowed options");
    options.add_options()
            ("help,h", "produce help message")
            ("version,v", "print the version number")
            ("seed,s", po::value<long unsigned int>(&parsedValues.seed), "game randomness seed")
            ;

    po::store(po::parse_command_line(argc, argv, options), rawValues);
    po::notify(rawValues);

    if(rawValues.count("help")){
        std::cout << options << '\n';
        exit(EXIT_SUCCESS);
    }
    if(rawValues.count("version")){
        printVersion();
        exit(EXIT_SUCCESS);
    }

    return std::make_pair(rawValues, parsedValues);
}

int main(int argc, const char* argv[]){
    auto [ rawValues, parsedValues ] = readArgs(argc, argv);

    auto heur1 = std::make_unique<PowerTimesToughnessHeuristic>(),
            heur2 = std::make_unique<PowerTimesToughnessHeuristic>();
    auto bar = std::make_shared<ProgressBar>();
    auto noop = std::make_shared<NoOpLogger>();
    auto pol1 = std::make_unique<UntilSomeoneDiesDFS>(noop, *heur1);
    auto pol2 = std::make_unique<UntilSomeoneDiesDFS>(bar, *heur2);
    auto ag1 = std::make_unique<SearchAgent>( 1, std::move(pol1), std::move(heur1) );
    auto ag2 = std::make_unique<SearchAgent>( 2, std::move(pol2), std::move(heur2) );

    Server_impl server("0.0.0.0", 8000);
    auto [ id, room ] = server.addRoom();
    std::cout << "Room ID is " << id << '\n';
    std::thread network_thread(&Server_impl::start, &server);

    std::array<std::unique_ptr<Agent>, 2> agents = { std::move(ag1), std::move(ag2) };

    Generator seed;
    if(rawValues.count("seed")) seed = Generator(parsedValues.seed);
    else seed = getRandom();
    std::cout << "Using seed " << seed << '\n';

    UnitsRepository repository;
    std::array<const Team*, 2> teams = { &repository.mkRandom(seed, 2), &repository.mkRandom(seed, 1) };

	Game game(teams, std::move(agents), seed);
	unsigned short int winner = game.play(&room, &std::cout);
    std::cout << winner << " won!\n";
    server.stop();
    network_thread.join();
}
