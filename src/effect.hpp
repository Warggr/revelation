#ifndef REVELATION_EFFECT_HPP
#define REVELATION_EFFECT_HPP

#include "step.hpp"
#include <memory>
#include <vector>
#include <string>

class State;

class Effect {
public:
	virtual ~Effect() = default;
	virtual std::unique_ptr<Step> resolve(State& state, unsigned int decision) = 0;
	virtual bool opponentChooses() const { return false; }
	virtual std::vector<std::string> getOptions();
};

/*
class OpponentEffect: public Effect {
	std::unique_ptr<Effect> effect;
public:
	OpponentEffect(std::unique_ptr<Effect> effect): effect(std::move(effect)) {};
	std::unique_ptr<Step> resolve(State& state, Agent& active, Agent& opponent){
		return effect->resolve(state, opponent, active);
	}
};*/

#endif // REVELATION_EFFECT_HPP
