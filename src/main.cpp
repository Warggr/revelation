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

int main(int argc, const char* argv[]){
    auto vm = readArgs(argc, argv);

    auto heur1 = std::make_unique<PowerTimesToughnessHeuristic>(),
            heur2 = std::make_unique<PowerTimesToughnessHeuristic>();
    auto noop = std::make_shared<NoOpLogger>();
    auto pol1 = std::make_unique<StaticDFS>(noop, *heur1);
    auto pol2 = std::make_unique<StaticDFS>(noop, *heur2);
    SearchAgent ag1( 1, std::move(pol1), std::move(heur1) );
    SearchAgent ag2( 2, std::move(pol2), std::move(heur2) );

    std::array<Agent*, NB_AGENTS> agents = { &ag1, &ag2 };

    GeneratorSeed seed = vm["seed"].as<GeneratorSeed>();
    std::cerr << "Using seed " << seed << '\n';

    Generator generatorForTeams(seed);
    UnitsRepository repository;
    std::filesystem::current_path(std::filesystem::current_path() / "resources");
    repository.mkDefaultTeams();
    for(int i = 0; i<30; i++){
        repository.addCharacter(ImmutableCharacter::random(generatorForTeams));
    }

    constexpr int NB_GAMES = 100;
    Generator generatorForGames(seed);
    for(int i = 0; i<NB_GAMES; i++) {
        std::cerr << "[" << i << "/" << NB_GAMES << "]";
        std::array<UnitsRepository::TeamList::iterator, 2> teams_iter = {repository.mkRandom(generatorForTeams, repository, ARMY_SIZE),
                                            repository.mkRandom(generatorForTeams, repository, ARMY_SIZE)};
        std::array<const Team*, 2> teams = { &teams_iter[0]->second, &teams_iter[1]->second };

        Game game(teams, agents, generatorForGames());
        auto gameInfo = game.play();
        for(int j=0; j<8; j++) std::cerr << '\b';
        std::cerr.flush();
        std::cout << gameInfo.whoWon;
        for(const auto& team : teams){
            for(const auto& row: team->characters) for(const auto& chr : row){
                std::cout << ',' << chr->slug;
            }
            std::cout << ",END";
        }
        std::cout << '\n';
        std::cout.flush();
    }
}
