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
    for(const auto& [ name, im ] : repository.getCharacters()){
        std::cout << im.slug << ':' << json(im).dump() << '\n';
    }
    std::cout << '\n';

    constexpr int NB_GAMES_PER_THREAD = 5000;
    constexpr int NB_THREADS = 1;
    constexpr int NB_GAMES = NB_THREADS * NB_GAMES_PER_THREAD;

    int completed = 0;
    std::mutex protectOutput;
    auto createDataSet = [&protectOutput,&completed,&repository,agents,NB_GAMES](GeneratorSeed seed){
        Generator generatorForGames(seed);
        Generator generatorForTeams(seed + 4);
        for(int i = 0; i<NB_GAMES_PER_THREAD; i++) {
            std::array<UnitsRepository::TeamList::iterator, 2> teams_iter = {
                    repository.mkRandom(generatorForTeams, repository, 1),
                    repository.mkRandom(generatorForTeams, repository, 1)
            };
            std::array<const Team*, 2> teams_ptr = { &teams_iter[0]->second, &teams_iter[1]->second };

            Game game(teams_ptr, agents, generatorForGames());
            auto gameInfo = game.play();
            protectOutput.lock();
            completed++;
            for(int j=0; j<11; j++) std::cerr << '\b';
            std::cerr << '[' << completed << '/' << NB_GAMES << ']';
            std::cerr.flush();
            std::cout << gameInfo.whoWon;
            for(const auto& team : teams_ptr){
                for(const auto& row: team->characters) for(const auto& chr : row){
                    std::cout << ',' << chr->slug;
                }
                std::cout << ",END";
            }
            std::cout << '\n';
            std::cout.flush();
            protectOutput.unlock();
            for(const auto& team : teams_iter)
                repository.deleteTeam( team );
        }
    };

    std::vector<std::thread> threads;
    for(int i = 0; i<NB_THREADS; i++){
        threads.emplace_back(createDataSet, seed+i);
    }
    for(auto& thread : threads) thread.join();
}
