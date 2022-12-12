#ifndef REVELATION_STEP_AGENT_HPP
#define REVELATION_STEP_AGENT_HPP

#include "agent.hpp"
#include <random>
#include <string_view>
#include <vector>
#include <utility>

class StepByStepAgent: public Agent {
    static const std::string_view chooseCharacterDefaultMessage;
protected:
    using OptionList = std::vector<std::pair<const char*, std::string_view>>;
    // TODO OPTIMIZE: very often we know the size of the final vector beforehand, we could already reserve it from the beginning
    class OptionListWrapper {
        OptionList data;
    public:
        static const char* const NULL_STRING; // = ""
        size_t add(std::string_view option, const char* keys = NULL_STRING){
            data.emplace_back(keys, option);
            return data.size() - 1;
        }
        OptionList get(){ return data; }
    };
    class OptionListWrapperWithStrings {
        std::vector<std::pair<const char*, std::string>> data;
    public:
        size_t add(std::string&& string, const char* keys = OptionListWrapper::NULL_STRING){
            data.emplace_back(keys, std::move(string));
            return data.size() - 1;
        }
        size_t add(std::string_view string, const char* keys = OptionListWrapper::NULL_STRING){
            return add(std::string(string), keys);
        }
        OptionList get(){
            OptionList retVal;
            for(auto& pair : data) retVal.emplace_back(pair.first, pair.second);
            return retVal;
        }
    };

    std::pair<const Character*, unsigned int> chooseCharacter(const State& state, OptionListWrapper& otherOptions, const std::string_view message = chooseCharacterDefaultMessage);
    std::pair<const Character*, unsigned int> chooseCharacter(const State& state, const std::string_view message = chooseCharacterDefaultMessage){
        OptionListWrapper list; return chooseCharacter(state, list, message);
    }
    static std::pair<uint, bool> inputValidation(const OptionList& list, const std::string_view& input);
    virtual uint choose(const OptionList& list, const std::string_view& message) = 0;
    static constexpr std::string_view SURRENDER = "!GG!";
public:
    StepByStepAgent(uint myId) : Agent(myId) {};
    const Team& getTeam(const UnitsRepository& repo) override;
    DiscardDecision getDiscard(const State&) override;
    MoveDecision getMovement(const State& state, unsigned int) override;
    ActionDecision getAction(const State& state) override;
    unsigned int getSpecialAction(const State& state, const Effect& effect) override;
};

class HumanAgent: public StepByStepAgent {
protected:
    uint choose(const OptionList& list, const std::string_view& message) override;
public:
    HumanAgent(uint myId);
};

class RandomAgent: public StepByStepAgent {
    std::minstd_rand generator;
protected:
    uint input(uint min, uint max);
    uint choose(const OptionList& list, const std::string_view&) override { return input(0, list.size() - 1); }
public:
    RandomAgent(uint myId);
};

#endif //REVELATION_STEP_AGENT_HPP
