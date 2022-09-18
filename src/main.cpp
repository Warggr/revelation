#include "game.hpp"
#include "agent.hpp"
#include "search/depthfirstsearch.hpp"
#include "search/loggers.hpp"
#include "random.hpp"
#include <array>
#include <iostream>

int main(){
    /*PowerTimesToughnessHeuristic heur;
    ProgressBar bar;
    NoOpLogger noop;
    AdaptiveDepthFirstSearch pol2(heur, noop);
    UntilSomeoneDiesDFS pol1(heur, bar);
    SearchAgent ag2( 1, pol2 );
    SearchAgent ag1( 1, pol1 );*/
    HumanAgent ag1(0);
    RandomAgent ag2(1);
    std::array<Agent*, 2> agents = { &ag1, &ag2 };

    //Generator seedForTeams = getRandom();
    std::array<Team, 2> teams = { mkEurope(), mkNearEast() };

    Generator seed = getRandom();

    std::cout << "Using seed " << seed << '\n';

    json j ( teams );

	Game game(std::move(teams), agents, seed);
	unsigned short int winner = game.play(true, true);
    std::cout << winner << " won!\n";
}
