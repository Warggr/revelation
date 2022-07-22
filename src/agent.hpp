#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

//from card import ActionCard
//from character import Character
#include "position.hpp"
#include "player.hpp"
#include "character.hpp"
#include "deck.hpp"

struct MoveDecision {
public:
    position from;
    position to;
    std::vector<Direction> moves;
};

class Agent {
public:
    uint8_t myId;

    Player* getMyPlayer(const State& state) const {
        return state.players[ myId ];
    }

    virtual ActionOrResource getDrawAction() = 0;
    virtual void onBegin(const State& state) {};
    virtual MoveDecision getMovement(const State& state) = 0;
    virtual AbilityDecision getAbility(const State& state) { return AbilityDecision(); };
    virtual ActionDecision getAction(const State& state) = 0;

/*    def drawAndDiscardStep(self):
        x : ActionOrResource = self.getDrawAction()
        if x == ActionOrResource.RESOURCES:
            self.resources += self.game.resDeck.draw()
            if len(self.resources) > MAX_RESOURCES:
                self.chooseAndDiscardResource()
        else:
            self.resources += self.actDeck.draw()
            if len(self.actions) > MAX_ACTIONS:
                self.chooseAndDiscardAction()
*/
};

#endif //REVELATION_AGENT_HPP
