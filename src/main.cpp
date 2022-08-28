#include "game.hpp"
#include "agent.hpp"
#include "search/depthfirstsearch.hpp"
#include <array>
#include <iostream>

int main(){
    HumanAgent ag1(0);
    //HumanAgent ag2(1);
    PowerTimesToughnessHeuristic heur;
    ProgressBar logger;
    auto* pol2 = new StaticDepthFirstSearch(heur, logger);
    pol2->setOpponentsTurn(new AdaptiveDepthFirstSearch(heur, logger));
    SearchAgent ag2( 1, pol2 );
    std::array<Agent*, 2> agents = { &ag1, &ag2 };
    std::array<Team, 2> teams = { mkEurope(), mkNearEast() };

	Game game(std::move(teams), agents);
	unsigned short int winner = game.play();
    std::cout << winner << " won!\n";
}
