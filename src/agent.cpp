#include "agent.hpp"
#include "state.hpp"
#include "constants.hpp"
#include <cassert>
#include <map>
#include <iostream>

std::ostream& operator<<(std::ostream& o, const position& pos){
    o << '[' << pos.column << ", " << pos.row << ']';
    return o;
}

uint HumanAgent::input(uint min, uint max){
    assert(min <= max and max < 100000); //too high values are usually an overflow
    while(true) {
        uint iSel; std::cin >> iSel;
        if(min <= iSel and iSel <= max) return iSel;
        else std::cout << "Please choose a number between " << min << " and " << max << "!\n";
    };
}

const Player& Agent::getMyPlayer(const State& state) const {
    return state.players[ myId ];
}

HumanAgent::HumanAgent(uint myId) : StepByStepAgent(myId) {
}

unsigned int StepByStepAgent::getSpecialAction(const State& state, Effect& effect) {
    std::vector<std::string> descriptions = effect.getOptions(state);
    for(unsigned int i=0; i<descriptions.size(); i++){
        ostream() << '[' << i << "]: " << descriptions[i] << '\n';
    }
    uint iSel = input(0, descriptions.size() - 1);
    return iSel;
}

const Character& StepByStepAgent::chooseCharacter(const State& state) {
    const Character* possibleValues[NB_CHARACTERS];
    unsigned int j = 0;
    for(uint i=0; i<NB_CHARACTERS; i++){
        if(not isDead(state.units[myId][i].get())){
            ostream() << '[' << j << "]: " << state.units[myId][i]->im.name << '\n';
            possibleValues[j] = state.units[myId][i].get();
            j += 1;
        }
    }

    ostream() << "Enter which character to select: ";
    uint iSel = input(1, j) - 1;
    return *possibleValues[iSel];
}

DiscardDecision StepByStepAgent::getDiscard(const State& state) {
    uint i = 0;
    for(const auto& card : getMyPlayer(state).getActions()) {
        ostream() << '[' << i++ << "]: " << to_string(card) << '\n';
    }
    for(const auto& card : getMyPlayer(state).getResourceCards()){
        i++;
        ostream() << '[' << i++ << "]: " << to_string(card) << '\n';
    }
    const uint actionsSize = getMyPlayer(state).getActions().size();
    ostream() << "Choose a card to discard: ";
    uint iSel = input(0, i-1);
    if(iSel < actionsSize)
        return DiscardDecision(true, iSel);
    else
        return DiscardDecision(false,iSel - actionsSize);
}

MoveDecision StepByStepAgent::getMovement(const State& state, unsigned int) {
    const Character& charSel = chooseCharacter(state);
    std::vector<MoveDecision> possibleMovs = state.allMovementsForCharacter(charSel);
    for(uint i = 0; i<possibleMovs.size(); i++)
        ostream() << '[' << i << "]: to " << possibleMovs[i].to << '\n';
    ostream() << "Enter which position to select or 0 to skip: ";
    uint iSel = input(0, possibleMovs.size());
    if(iSel == 0) return MoveDecision::pass();
    else {
        MoveDecision movSel = possibleMovs[iSel-1];
        return movSel;
    }
}

ActionDecision StepByStepAgent::getAction(const State& state) {
    ActionDecision ret{};
    const std::vector<ActionCard>& cards = getMyPlayer(state).getActions();
    if(cards.empty())
        return ActionDecision::pass();
    for(uint i=0; i<cards.size(); i++){
        ostream() << '[' << (i+1) << "]: " << to_string(cards[i]) << '\n';
    }
    ostream() << "Choose a card, any card (or 0 to skip): ";
    uint iSel = input(0, cards.size());

    if(iSel == 0)
        return ActionDecision::pass();
    ret.card = cards[iSel - 1];
    if(ret.card == ActionCard::DEFENSE){
        ret.subjectPos = chooseCharacter(state).pos;
    } else {
        auto allPossibleAttacks = state.allAttacks();
        if(allPossibleAttacks.empty())
            return ActionDecision::pass();
        std::vector<std::array<const Character*, 2>> array;
        for(const auto& [ unit, enemies ] : allPossibleAttacks){
            ostream() << unit->im.name << '\n';
            for(const auto& enemy : enemies){
                array.push_back( { unit, enemy } );
                const char* startCharacter = (enemy == enemies.back()) ? "└" : "├";
                ostream() << '[' << array.size() << ']' << startCharacter << "─" << enemy->im.name << '\n';
            }
        }
        ostream() << "Enter which attack to select: (or 0 to skip)";
        uint iSel = input(0, array.size());
        if(iSel == 0) return ActionDecision::pass();
        ret.subjectPos = array[iSel - 1][0]->pos;
        ret.objectPos = array[iSel - 1][1]->pos;
    }
    return ret;
}

RandomAgent::RandomAgent(uint myId): StepByStepAgent(myId), generator(std::random_device()()) {
}

uint RandomAgent::input(uint min, uint max){
    assert(min <= max and max < 100000); //too high values are usually an overflow
    return std::uniform_int_distribution<uint>(min, max)(generator);
}
