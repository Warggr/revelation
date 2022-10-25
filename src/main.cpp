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
    /*PowerTimesToughnessHeuristic heur;
    ProgressBar bar;
    NoOpLogger noop;
    AdaptiveDepthFirstSearch pol2(heur, noop);
    UntilSomeoneDiesDFS pol1(heur, bar);
    SearchAgent ag2( 1, pol2 );
    SearchAgent ag1( 1, pol1 );
    RandomAgent ag1(0);
    RandomAgent ag2(1);*/
    auto ag1 = std::make_unique<HumanAgent>(0);
    Server_impl server("0.0.0.0", 8000);
    auto [ id, room ] = server.addRoom();
    std::cout << "Room ID is " << id << '\n';

    std::thread network_thread(&Server_impl::start, &server);

    auto networkAgents = NetworkAgent::makeAgents(1, room, 1);
    std::array<std::unique_ptr<Agent>, 2> agents = { std::move(ag1), std::move(networkAgents[0]) };

    UnitsRepository repository;
    std::array<const Team*, 2> teams = { &repository.mkEurope(), &repository.mkNearEast() };

    Generator seed = getRandom();

    std::cout << "Using seed " << seed << '\n';

	Game game(teams, std::move(agents), seed);
	unsigned short int winner = game.play(&room, true);
    std::cout << winner << " won!\n";
    server.stop();
    network_thread.join();
}
