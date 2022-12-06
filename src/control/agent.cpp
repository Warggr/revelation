#include "agent.hpp"
#include "gameplay/state.hpp"
#include "setup/units_repository.hpp"
#include "setup/team.hpp"
#include "constants.hpp"
#include <cassert>
#include <map>
#include <iostream>
#include <charconv>
#include <cstring>

const char* const StepByStepAgent::OptionListWrapper::NULL_STRING = "";
constexpr std::string_view SKIP = "!Skip";
constexpr std::string_view BACK = "!Back";
const char* const SKIP_KEYS = ">";
const char* const BACK_KEYS = "<";
const char* const AGAIN_KEYS = "?";

std::pair<uint, bool> StepByStepAgent::inputValidation(const OptionList& options, const std::string_view& input){
    int int_val = 0;
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
showOptions:
    for(uint i = 0; i<options.size(); i++){
        std::cout << '[' << i;
        if(*options[i].first != 0) std::cout << '/' << options[i].first;
        std::cout << "]:" << options[i].second << '\n';
    }
    std::cout << message << ": ";
    while(true) {
        char buffer[5]; std::cin.getline(buffer, 5);
        if(strcmp(buffer, AGAIN_KEYS) == 0) goto showOptions;
        if(strcmp(buffer, SURRENDER.data()) == 0) throw AgentSurrenderedException(myId);
        auto [ value, success ] = StepByStepAgent::inputValidation(options, buffer);
        if(success) return value;
        else std::cout << "Please choose a number between 0 and " << options.size() << "!\n";
    }
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

std::pair<const Character*, unsigned int> StepByStepAgent::chooseCharacter(const State &state, OptionListWrapper& otherOptions){
    char keys[NB_CHARACTERS][2];
    const Character* possibleValues[NB_CHARACTERS];
    unsigned int charactersOffset = otherOptions.get().size();

    unsigned int j = 0;
    for(uint i=0; i<NB_CHARACTERS; i++){
        if(not isDead(state.units[myId][i].get())){
            keys[i][0] = state.units[myId][i]->uid; keys[i][1] = 0;
            otherOptions.add(state.units[myId][i]->im.name, keys[i]);
            possibleValues[j++] = state.units[myId][i].get();
        }
    }
    unsigned int iSel = choose(otherOptions.get(), "Choose a character");
    if(iSel < charactersOffset)
        return std::make_pair( nullptr, iSel );
    else
        return std::make_pair( possibleValues[iSel - charactersOffset], iSel );
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

std::string to_string(const MoveDecision& mov){
    std::stringstream str; //an ugly way to create a string by using the operator<<
    for(const auto& direction : mov.moves)
        str << to_symbol(direction);
    str << " to " << mov.to;
    return str.str();
}

MoveDecision StepByStepAgent::getMovement(const State& state, unsigned int) {
chooseCharacter:
    const Character* charSel;
    {
        OptionListWrapper options;
        auto skip = options.add(SKIP, SKIP_KEYS);
        unsigned int iSel;
        std::tie(charSel, iSel) = chooseCharacter(state, options);
        if (iSel == skip) return MoveDecision::pass();
    }
    std::vector<MoveDecision> possibleMovs = state.allMovementsForCharacter(*charSel);

    OptionListWrapperWithStrings options;
    const auto skip_pos = options.add(SKIP);
    const auto back_pos = options.add(BACK);
    constexpr auto OFFSET = 2;
    for(const auto& mov : possibleMovs){
        options.add(to_string(mov));
    }
    uint iSel = choose(options.get(), "Enter which position to select");

    if(iSel == back_pos) goto chooseCharacter;
    else if(iSel == skip_pos) return MoveDecision::pass();
    else return possibleMovs[iSel-OFFSET];
}

ActionDecision StepByStepAgent::getAction(const State& state) {
    ActionDecision ret{};
    const std::vector<ActionCard>& cards = getMyPlayer(state).getActions();
    if(cards.empty())
        return ActionDecision::pass();

chooseCard:
    { //to limit the scope of @var options and @var iSel
        OptionListWrapper options;
        options.add(SKIP, SKIP_KEYS);
        for(auto card: cards) {
            options.add(to_string(card));
        }
        uint iSel = choose(options.get(), "Choose a card, any card (or 0 to skip)");

        if(iSel == 0)
            return ActionDecision::pass();
        ret.card = cards[iSel - 1];
    }

    if(ret.card == ActionCard::DEFENSE){
        OptionListWrapper options;
        options.add(SKIP, SKIP_KEYS); options.add(BACK, BACK_KEYS);
        auto [ c, iSel ] = chooseCharacter(state, options);
        if(c == nullptr){
            if(iSel == 0) return ActionDecision::pass();
            else if(iSel == 1) goto chooseCard;
            else assert(false);
        }
        ret.subjectPos = c->pos;
    } else {
        auto allPossibleAttacks = state.allAttacks();
        if(allPossibleAttacks.empty())
            return ActionDecision::pass();
        std::vector<std::array<const Character*, 2>> array;
        OptionListWrapperWithStrings options;
        options.add(SKIP, SKIP_KEYS); options.add(BACK, BACK_KEYS);
        for(const auto& [ unit, enemies ] : allPossibleAttacks){
            for(const auto& enemy : enemies){
                array.push_back( { unit, enemy } );
                options.add(std::string(unit->im.name) + " -> " + enemy->im.name);
            }
        }
        uint iSel = choose(options.get(), "Enter which attack to select: (or 0 to skip)");
        if(iSel == 0) return ActionDecision::pass();
        else if(iSel == 1) goto chooseCard;
        constexpr int OFFSET = 2;
        ret.subjectPos = array[iSel - OFFSET][0]->pos;
        ret.objectPos = array[iSel - OFFSET][1]->pos;
    }
    return ret;
}

const Team& StepByStepAgent::getTeam(const UnitsRepository& repo) {
    OptionListWrapper options;
    std::vector<const Team*> backer;
    for(const auto& team : repo.getTeams()) {
        options.add(team.first); // name of the team
        backer.push_back(&team.second);
    }
    unsigned int iSel = choose(options.get(), "Choose a team");
    return *backer[iSel];
}

RandomAgent::RandomAgent(uint myId): StepByStepAgent(myId), generator(std::random_device()()) {
}

uint RandomAgent::input(uint min, uint max){
    assert(min <= max and max < 100000); //too high values are usually an overflow
    return std::uniform_int_distribution<uint>(min, max)(generator);
}
