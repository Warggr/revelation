#ifndef REVELATION_EFFECT_HPP
#define REVELATION_EFFECT_HPP

#include "step.hpp"
#include "decision.hpp"
#include <memory>
#include <vector>
#include <string>

class State; struct Character;

// abstract / interface
class Effect {
public:
	virtual ~Effect() = default;
	virtual std::unique_ptr<Step> resolve(State& state, unsigned int decision) = 0;
	virtual bool opponentChooses() const { return false; }
	virtual std::vector<std::string> getOptions(const State& state) const = 0;
};

/*
// abstract / interface
class SpecialCharacterAbility {
public:
	virtual ~SpecialCharacterAbility() = default;
	virtual std::unique_ptr<Step> resolve(State& state, unsigned int decision, unsigned short int iActive, Character* owner) = 0;
	virtual bool opponentChooses() const { return false; }
	virtual std::vector<std::string> getOptions(const State& state) const;
};

template<typename SCAbility>
class SpecialCharacterEffect : public Effect {
	SCAbility ability;
	Character* owner;
	unsigned short int iActive;
public:
	SpecialCharacterEffect(SCAbility ability, Character* owner, unsigned short int iActive):
		ability(ability), owner(owner), iActive(iActive) {};
	std::unique_ptr<Step> resolve(State& state, unsigned int decision) override {
		return ability.resolve(state, decision, iActive, owner);
	}
	bool opponentChooses() const override { return ability.opponentChooses(); }
	std::vector<std::string> getOptions(const State& state) const override { return ability.getOptions(state); }
};
*/

#endif // REVELATION_EFFECT_HPP
