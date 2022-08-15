#include "agent.hpp"
#include "state.hpp"
#include "constants.hpp"
#include <map>
#include <iostream>

std::ostream& operator<<(std::ostream& o, const position& pos){
    o << '[' << pos.column << ", " << pos.row << ']';
    return o;
}

uint input(uint min, uint max){
    while(true) {
        uint iSel; std::cin >> iSel;
        if(min <= iSel and iSel <= max) return iSel;
        else std::cout << "Please choose a number between " << min << " and " << max << "!\n";
    };
}

const Player& Agent::getMyPlayer(const State& state) const {
    return state.players[ myId ];
}

HumanAgent::HumanAgent(uint myId) : Agent(myId) {
    //name = input("Hi! Please enter your name: ")
}

const Character& HumanAgent::chooseCharacter(const State& state) const {
    for(uint i=0; i<NB_CHARACTERS; i++)
        if(not isDead(state.units[myId][i].pt()))
            std::cout << '[' << i << "]: " << state.units[myId][i]->name << '\n';

    std::cout << "Enter which character to select: ";
    uint iSel = input(0, NB_CHARACTERS-1);
    return *state.units[myId][iSel];
}

ActionOrResource HumanAgent::getDrawAction(const State&) {
    std::cout << "Choose [1] draw action or [2] draw resource: ";
    uint iSel = input(0, 1);
    return (iSel == 1) ? ActionOrResource::ACTION : ActionOrResource::RESOURCES;
}

MoveDecision HumanAgent::getMovement(const State& state, unsigned int) {
    const Character& charSel = chooseCharacter(state);
    std::vector<MoveDecision> possibleMovs = state.allMovementsForCharacter(charSel);
    for(uint i = 0; i<possibleMovs.size(); i++)
        std::cout << '[' << i << "]: to " << possibleMovs[i].to << '\n';
    std::cout << "Enter which position to select: ";
    uint iSel = input(0, possibleMovs.size() - 1);
    MoveDecision movSel = possibleMovs[iSel];
    return movSel;
}

ActionDecision HumanAgent::getAction(const State& state) {
    ActionDecision ret{};
    const std::vector<ActionCard>& cards = getMyPlayer(state).getActions();
    if(cards.empty())
        return ActionDecision::pass();
    for(uint i=0; i<cards.size(); i++){
        std::cout << '[' << (i+1) << "]: " << cards[i] << '\n';
    }
    std::cout << "Choose a card, any card (or 0 to skip): ";
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
        int i = 1;
        for(const auto& [ unit, enemies ] : allPossibleAttacks){
            std::cout << unit->name << '\n';
            for(const auto& enemy : enemies){
                const char* startCharacter = (enemy == enemies.back()) ? "└" : "├";
                std::cout << '[' << i << ']' << startCharacter << "─" << enemy->name << '\n';
                array.push_back( { unit, enemy } );
                i += 1;
            }
        }
        std::cout << "Enter which attack to select: ";
        uint iSel = input(0, i-1);
        ret.subjectPos = array[iSel - 1][0]->pos;
        ret.objectPos = array[iSel - 1][1]->pos;
    }
    return ret;
}
