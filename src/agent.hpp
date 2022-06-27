#ifndef REVELATION_AGENT_H
#define REVELATION_AGENT_H

//from card import ActionCard
//from character import Character
#include "position.hpp"
#include "character.hpp"

enum ActionOrResource { ACTION, RESOURCES };

class ActionCard{

};

struct MoveDecision {
    position from;
    position to;
};

struct ActionDecision {
    ActionCard* card;
    Character* subject;
    Character* object;
};

struct AbilityDecision {};

/*
class Agent represents a decision-maker. It is an abstract class that can be implemented by an UI that interfaces with a human, or by an AI.
*/
class Agent {
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

#endif //REVELATION_AGENT_H
