#include "agent.hpp"
#include "gameplay/state.hpp"
#include "constants.hpp"
#include <cassert>
#include <map>
#include <iostream>
#include <charconv>

const char* const StepByStepAgent::OptionListWrapper::NULL_STRING = "";
const char* const SKIP = "Skip";

std::ostream& operator<<(std::ostream& o, const position& pos){
    o << '[' << pos.column << ", " << pos.row << ']';
    return o;
}

std::pair<uint, bool> StepByStepAgent::inputValidation(const OptionList& options, const std::string_view& input){
    int int_val;
    auto result = std::from_chars(input.data(), input.data() + input.size(), int_val);
    if(result.ec != std::errc::invalid_argument) { // valid int
        if(int_val >= 0){
            uint uint_val = static_cast<uint>(int_val);
            if(uint_val < options.size()) return std::make_pair(uint_val, true);
        }
    } else if(input.size() == 1){
        for(uint i = 0; i < options.size(); i++) {
            for(const char *c = options[i].first; *c != 0; c++){
                if (*c == input[0])
                    return std::make_pair(i, true);
            }
        }
    }
    return std::make_pair(0, false);
}

uint HumanAgent::choose(const OptionList& options, const std::string_view& message){
    for(uint i = 0; i<options.size(); i++){
        std::cout << '[' << i;
        if(*options[i].first != 0) std::cout << '/' << options[i].first;
        std::cout << "]:" << options[i].second << '\n';
    }
    std::cout << message << ": ";
    while(true) {
        char buffer[5]; std::cin.getline(buffer, 5);
        auto [ value, success ] = StepByStepAgent::inputValidation(options, buffer);
        if(success) return value;
        else std::cout << "Please choose a number between 0 and " << options.size() << "!\n";
    }
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
    OptionListWrapper options;
    for(const auto& desc : descriptions){
        options.add(desc);
    }
    uint iSel = choose(options.get(), "Choose a special action");
    return iSel;
}

const Character& StepByStepAgent::chooseCharacter(const State& state) {
    const Character* possibleValues[NB_CHARACTERS];
    char keys[NB_CHARACTERS][2];
    OptionListWrapper options;
    unsigned int j = 0;
    for(uint i=0; i<NB_CHARACTERS; i++){
        if(not isDead(state.units[myId][i].get())){
            keys[i][0] = state.units[myId][i]->uid; keys[i][1] = 0;
            options.add(state.units[myId][i]->im.name, keys[i]);
            possibleValues[j] = state.units[myId][i].get();
            j += 1;
        }
    }

    uint iSel = choose(options.get(), "Enter which character to select");
    return *possibleValues[iSel];
}

DiscardDecision StepByStepAgent::getDiscard(const State& state) {
    OptionListWrapper options;
    for(const auto& card : getMyPlayer(state).getActions()) {
        options.add(to_string(card));
    }
    for(const auto& card : getMyPlayer(state).getResourceCards()){
        options.add(to_string(card));
    }
    const uint actionsSize = getMyPlayer(state).getActions().size();
    uint iSel = choose(options.get(), "Choose a card to discard");
    if(iSel < actionsSize)
        return DiscardDecision(true, iSel);
    else
        return DiscardDecision(false,iSel - actionsSize);
}

MoveDecision StepByStepAgent::getMovement(const State& state, unsigned int) {
    const Character& charSel = chooseCharacter(state);
    std::vector<MoveDecision> possibleMovs = state.allMovementsForCharacter(charSel);
    OptionListWrapperWithStrings options;
    options.add(SKIP);
    for(uint i = 0; i<possibleMovs.size(); i++){
        std::stringstream str; //an ugly way to create a string by using the operator<<
        for(const auto& direction : possibleMovs[i].moves)
            str << to_symbol(direction);
        str << " to " << possibleMovs[i].to;
        options.add(str.str());
    }
    uint iSel = choose(options.get(), "Enter which position to select or 0 to skip");
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
    { //to limit the scope of @var options and @var iSel
        OptionListWrapper options;
        options.add(SKIP);
        for(auto card: cards) {
            options.add(to_string(card));
        }
        uint iSel = choose(options.get(), "Choose a card, any card (or 0 to skip)");

        if(iSel == 0)
            return ActionDecision::pass();
        ret.card = cards[iSel - 1];
    }
    if(ret.card == ActionCard::DEFENSE){
        ret.subjectPos = chooseCharacter(state).pos;
    } else {
        auto allPossibleAttacks = state.allAttacks();
        if(allPossibleAttacks.empty())
            return ActionDecision::pass();
        std::vector<std::array<const Character*, 2>> array;
        OptionListWrapperWithStrings options;
        options.add(SKIP);
        for(const auto& [ unit, enemies ] : allPossibleAttacks){
            for(const auto& enemy : enemies){
                array.push_back( { unit, enemy } );
                options.add(std::string(unit->im.name) + " -> " + enemy->im.name);
            }
        }
        uint iSel = choose(options.get(), "Enter which attack to select: (or 0 to skip)");
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
