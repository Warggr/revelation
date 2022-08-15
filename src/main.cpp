#include "game.hpp"
#include "agent.hpp"
#include "search/depthfirstsearch.hpp"
#include <array>

std::array<Team, 2> teams = {
    {
        {
            "Near East",
            {
                {
                    {
                        Character("Mounted archers", 60, 30, 10, 3, 3, 534.90, true),
                        Character("Captives", 20, 20, 10, 2, 1, 288.70),
                        Character("Saracens", 80, 50, 20, 2, 1, 454.90, "Defense 20(light)")
                    },
                    {
                        Character("Canons", 60, 70, 0, 1, 4, 551.60, false),
                        Character("Arab officer", 100, 10, 10, 2, 1, 355.30),
                        Character("Crossbowman", 40, 40, 20, 2, 3, 489.10, false),
                    }
                }
            }
        },{
            "Europe",
            {
                {
                    {
                        Character("Crossbowman"        , 40, 40, 20, 2, 3, 489.10, false ),
                        Character("Armored knight"     , 100, 60, 30, 1, 1, 449.90, "Defense 20(light)" ),
                        Character("Fanatics"           , 20, 20, 10, 2, 1, 288.70 ),
                    },
                    {
                        Character("Fanatics"           , 20, 20, 10, 2, 1, 288.70 ),
                        Character("Knight"             , 90, 50, 10, 3, 1, 520.50, "Defense 20(light)" ),
                        Character("Lord officer"       , 100, 10, 10, 2, 1, 355.30)
                    }
                }
            }
        }
    }
};

int main(){
    HumanAgent ag1(0);
    //HumanAgent ag2(1);
    PowerTimesToughnessHeuristic heur;
    ProgressBar logger;
    auto* pol2 = new StaticDepthFirstSearch(heur, logger);
    pol2->setOpponentsTurn(new AdaptiveDepthFirstSearch(heur, logger));
    SearchAgent ag2( 1, pol2 );
    std::array<Agent*, 2> agents = { &ag1, &ag2 };

	Game game(teams, agents);
	game.play();
}
