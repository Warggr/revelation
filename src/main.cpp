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
    Server server("0.0.0.0", 8000);
    ServerRoom& room = server.addRoom().second;

    std::thread network_thread(&Server::start, &server);

    auto networkAgents = NetworkAgent::makeAgents(2, room);
    std::array<std::unique_ptr<Agent>, 2> agents = { std::move(networkAgents[0]), std::move(networkAgents[1]) };

    //Generator seedForTeams = getRandom();
    std::array<Team, 2> teams = { mkEurope(), mkNearEast() };

    Generator seed = getRandom();

    std::cout << "Using seed " << seed << '\n';

	Game game(std::move(teams), std::move(agents), seed);
	unsigned short int winner = game.play(&room, true);
    std::cout << winner << " won!\n";
    server.stop();
    network_thread.join();
}
