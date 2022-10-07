#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

#include "gameplay/player.hpp"
#include "gameplay/character.hpp"
#include "gameplay/deck.hpp"
#include "gameplay/decision.hpp"
#include "position.hpp"
#include <iostream>
#include <random>
#include <exception>

class State;

struct AgentSurrenderedException: public std::exception{
    uint id;
    AgentSurrenderedException(uint id): id(id){};
};

class Agent {
protected:
    uint myId : 1;

    const Player& getMyPlayer(const State& state) const;

    void surrender(){ throw AgentSurrenderedException(myId); }
public:
    Agent(unsigned int myId) : myId(myId) {};
    virtual ~Agent() = default;
    virtual void onBegin(const State&) {};
    virtual DiscardDecision getDiscard(const State& state) = 0;
    virtual MoveDecision getMovement(const State& state, unsigned nb) = 0;
    virtual AbilityDecision getAbility(const State&) { return {}; };
    virtual ActionDecision getAction(const State& state) = 0;
    virtual unsigned int getSpecialAction(const State& state, Effect& effect) = 0;
};

class StepByStepAgent: public Agent {
    const Character& chooseCharacter(const State& state);
protected:
    using OptionList = std::vector<std::pair<const char*, std::string_view>>;
    // TODO OPTIMIZE: very often we know the size of the final vector beforehand, we could already reserve it from the beginning
    class OptionListWrapper {
        OptionList data;
    public:
        static const char* const NULL_STRING; // = ""
        void add(std::string_view option, const char* keys = NULL_STRING){
            data.emplace_back(keys, option);
        }
        OptionList get(){ return data; }
    };
    class OptionListWrapperWithStrings {
        std::vector<std::pair<const char*, std::string>> data;
    public:
        void add(std::string&& string, const char* keys = OptionListWrapper::NULL_STRING){
            data.emplace_back(keys, std::move(string));
        }
        OptionList get(){
            OptionList retVal;
            for(auto& pair : data) retVal.emplace_back(pair.first, pair.second);
            return retVal;
        }
    };
    static std::pair<uint, bool> inputValidation(const OptionList& list, const std::string_view& input);
    virtual uint choose(const OptionList& list, const std::string_view& message) = 0;
    virtual uint input(uint min, uint max) = 0;
public:
    StepByStepAgent(uint myId) : Agent(myId) {};
    DiscardDecision getDiscard(const State&) override;
    MoveDecision getMovement(const State& state, unsigned int) override;
    ActionDecision getAction(const State& state) override;
    unsigned int getSpecialAction(const State& state, Effect& effect) override;
};

class HumanAgent: public StepByStepAgent {
protected:
    uint input(uint min, uint max) override;
    uint choose(const OptionList &list, const std::string_view &message) override;
public:
    HumanAgent(uint myId);
};

class RandomAgent: public StepByStepAgent {
    std::minstd_rand generator;
protected:
    uint input(uint min, uint max) override;
    uint choose(const OptionList& list, const std::string_view&) override { return input(0, list.size()); }
public:
    RandomAgent(uint myId);
};

#endif //REVELATION_AGENT_HPP
