#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

#include "gameplay/player.hpp"
#include "gameplay/character.hpp"
#include "gameplay/deck.hpp"
#include "gameplay/decision.hpp"
#include "setup/team.hpp"
#include "setup/units_repository.hpp"
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
    virtual void sync_init(){};

    virtual const Team& getTeam(const UnitsRepository& repo) = 0;

    virtual void onBegin(const State&) {};
    virtual DiscardDecision getDiscard(const State& state) = 0;
    virtual MoveDecision getMovement(const State& state, unsigned nb) = 0;
    virtual AbilityDecision getAbility(const State&) { return {}; };
    virtual ActionDecision getAction(const State& state) = 0;
    virtual unsigned int getSpecialAction(const State& state, Effect& effect) = 0;
};

class StepByStepAgent: public Agent {
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

    std::pair<const Character*, unsigned int> chooseCharacter(const State& state, OptionListWrapper& otherOptions);
    std::pair<const Character*, unsigned int> chooseCharacter(const State& state){
        OptionListWrapper list; return chooseCharacter(state, list);
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
    unsigned int getSpecialAction(const State& state, Effect& effect) override;
};

class HumanAgent: public StepByStepAgent {
protected:
    uint choose(const OptionList &list, const std::string_view &message) override;
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

#endif //REVELATION_AGENT_HPP
