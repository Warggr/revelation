#include "agent.hpp"
#include "state.hpp"
#include "constants.hpp"
#include <map>
#include "iostream"

std::ostream& operator<<(std::ostream& o, const position& pos){
    o << '[' << pos.column << ", " << pos.row << ']';
    return o;
}

const Player& Agent::getMyPlayer(const State& state) const {
    return state.players[ myId ];
}


HumanAgent::HumanAgent() {
    //name = input("Hi! Please enter your name: ")
}

const character& HumanAgent::chooseCharacter(const State& state) const {
    for(uint i=0; i<NB_CHARACTERS; i++)
        if(not isDead(state.units[myId][i])){
            std::cout << '[' << i << "]: " << state.units[myId][i]->name << '\n';
    }

    std::cout << "Enter which character to select: \n";
    int iSel = 0; std::cin >> iSel;
    return *state.units[myId][iSel];
}

ActionOrResource HumanAgent::getDrawAction(const State&) {
    std::cout << "Choose [1] draw action or [2] draw resource: ";
    int iSel; std::cin >> iSel;
    return (iSel == 1) ? ActionOrResource::ACTION : ActionOrResource::RESOURCES;
}

MoveDecision HumanAgent::getMovement(const State& state, unsigned int) {
    const character& charSel = chooseCharacter(state);
    std::vector<MoveDecision> possibleMovs = state.allMovementsForCharacter(charSel);
    for(uint i = 0; i<possibleMovs.size(); i++)
        std::cout << '[' << i << "]: to " << possibleMovs[i].to << '\n';
    std::cout << "Enter which position to select: \n";
    uint iSel; std::cin >> iSel;
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
    std::cout << "Choose a card, any card (or 0 to skip):\n";
    uint iSel; std::cin >> iSel;
    if(iSel == 0)
        return ActionDecision::pass();
    ret.card = cards[iSel - 1];
    if(ret.card == ActionCard::DEFENSE){
        ret.subjectPos = chooseCharacter(state).pos;
    } else {
        std::map<character*, std::vector<character*>> allPossibleAttacks = state.allAttacks();
        if(allPossibleAttacks.empty())
            return ActionDecision::pass();
        std::vector<std::array<const character*, 2>> array;
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
        uint iSel; std::cin >> iSel;
        ret.subjectPos = array[iSel - 1][0]->pos;
        ret.objectPos = array[iSel - 1][1]->pos;
    }
    return ret;
}
