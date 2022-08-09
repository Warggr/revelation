#include "game.hpp"
#include <array>

std::array<Team, 2> teams = {
    {
        {
            "Near East",
            {
                {
                    {
                        character("Mounted archers", 60, 30, 10, 3, 3, 534.90, true),
                        character("Captives", 20, 20, 10, 2, 1, 288.70),
                        character("Saracens", 80, 50, 20, 2, 1, 454.90, "Defense 20(light)")
                    },
                    {
                        character("Canons", 60, 70, 0, 1, 4, 551.60, false),
                        character("Arab officer", 100, 10, 10, 2, 1, 355.30),
                        character("Crossbowman", 40, 40, 20, 2, 3, 489.10, false),
                    }
                }
            }
        },{
            "Europe", {
            {
                {
                    character("Mounted archers", 60, 30, 10, 3, 3, 534.90, true),
                    character("Captives", 20, 20, 10, 2, 1, 288.70),
                    character("Saracens", 80, 50, 20, 2, 1, 454.90, "Defense 20(light)")
                }, {
                    character("Canons", 60, 70, 0, 1, 4, 551.60, false),
                    character("Arab officer", 100, 10, 10, 2, 1, 355.30),
                    character("Crossbowman", 40, 40, 20, 2, 3, 489.10, false),
                }
            }
            }
        }
    }
};

int main(){
	std::array<Agent*, 2> agents = {
		new HumanAgent(),
		new HumanAgent()
	};

	Game game(teams, agents);
	game.play();
}
