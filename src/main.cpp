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

namespace po = boost::program_options;

using json = nlohmann::json;

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
    auto noop = std::make_shared<NoOpLogger>();
    auto pol1 = std::make_unique<StaticDFS>(noop, *heur1);
    auto pol2 = std::make_unique<StaticDFS>(noop, *heur2);
    SearchAgent ag1( 1, std::move(pol1), std::move(heur1) );
    SearchAgent ag2( 2, std::move(pol2), std::move(heur2) );

    std::array<Agent*, NB_AGENTS> agents = { &ag1, &ag2 };

    GeneratorSeed seed = rawValues.count("seed")
        ? parsedValues.seed
        : getRandom();
    std::cout << "Using seed " << seed << '\n';

    UnitsRepository repository;
    std::array<const Team*, 2> teams = { &repository.mkRandom(seed, 6), &repository.mkRandom(seed, 6) };

	Game game(teams, std::move(agents), seed);
    game.logger.addSubLogger<FileLogger>(std::cout).addSubLogger<LiveServerAndLogger>(room);
	auto gameInfo = game.play();
    std::cout << gameInfo.whoWon << " won!\n";
    server.stop();
    network_thread.join();
}
