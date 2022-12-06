#include "control/game.hpp"
#include "control/agent.hpp"
#include "search/depthfirstsearch.hpp"
#include "search/loggers.hpp"
#include "random.hpp"
#include "network/server.hpp"
#include "network/network_agent.hpp"
#include <array>
#include <iostream>
#include <thread>

int main(){
    auto heur1 = std::make_unique<PowerTimesToughnessHeuristic>(),
            heur2 = std::make_unique<PowerTimesToughnessHeuristic>();
    auto bar = std::make_shared<ProgressBar>();
    auto noop = std::make_shared<NoOpLogger>();
    auto pol1 = std::make_unique<StaticDFS>(noop, *heur1);
    auto pol2 = std::make_unique<StaticDFS>(bar, *heur2);
    auto ag1 = std::make_unique<SearchAgent>( 1, std::move(pol1), std::move(heur1) );
    auto ag2 = std::make_unique<SearchAgent>( 2, std::move(pol2), std::move(heur2) );
    Server_impl server("0.0.0.0", 8000);
    auto [ id, room ] = server.addRoom();
    std::cout << "Room ID is " << id << '\n';

    std::thread network_thread(&Server_impl::start, &server);

    std::array<std::unique_ptr<Agent>, 2> agents = { std::move(ag1), std::move(ag2) };

    UnitsRepository repository;
    std::array<const Team*, 2> teams = { &repository.mkEurope(), &repository.mkNearEast() };

    Generator seed = getRandom();

    std::cout << "Using seed " << seed << '\n';

	Game game(teams, std::move(agents), seed);
	unsigned short int winner = game.play(&room, &std::cout);
    std::cout << winner << " won!\n";
    server.stop();
    network_thread.join();
}
