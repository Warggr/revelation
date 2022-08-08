//
// Created by Diana Amirova on 08.07.22.
//

#ifndef REVELATION_STEP_HPP
#define REVELATION_STEP_HPP

#include "string"
#include "constants.hpp"
#include "deck.hpp"
#include "nlohmann/json.hpp"
#include "position.hpp"
#include "character.hpp"
#include <variant>
#include <memory>

using json = nlohmann::json;

template<typename T>
using uptr = std::unique_ptr<T>;

class Step {
public:
    virtual ~Step() = default;
    virtual json to_json(nlohmann::basic_json<> &j, const Step &step) = 0;
    virtual bool isPass() const = 0;
};

class BeginStep : public Step{
public:
    bool isPass() const override { return false; }
    json to_json(nlohmann::basic_json<> &j, const Step &step) override;
};

class DrawStep : public Step {
    std::variant<ActionCard, Faction> cardDrawn;
    std::tuple<int, int> size;
public:
    DrawStep(std::variant<ActionCard, Faction> cardDrawn, std::tuple<int, int> size){
        this->cardDrawn = cardDrawn;
        this->size = size;
    }
    bool isPass() const override { return false; }
    json to_json(nlohmann::basic_json<> &j, const Step &step) override;
};

class MoveStep : public Step {
    position from;
    position to;
    char uid;
    std::vector<Direction> moves;
    int firstCOF;

public:
    MoveStep(position from, position to, char uid, std::vector<Direction> moves, int firstCOF) : from(from), to(to), uid(uid) {
        this->moves = moves;
        this->firstCOF = firstCOF;
    }
    static MoveStep pass() { return { {}, {}, '0', {}, 0 }; }
    bool isPass() const override { return moves.empty(); }
    json to_json(nlohmann::basic_json<> &j, const Step &step) override;
};

class AbilityStep : public Step {
public:
    bool isPass() const override { return true; }
    json to_json(nlohmann::basic_json<> &j, const Step &step) override;
};

class ActionStep final: public Step {
    ActionCard cardLost;
    position subject, object;
    unsigned int setHP;
    unsigned int lostHP;
public:
    bool del = false;
    constexpr ActionStep(ActionCard cardLost, position subject, position object, unsigned int setHP, int diffHP):
        cardLost(cardLost), subject(subject), object(object), setHP(setHP), lostHP(diffHP) {};
    ~ActionStep() final = default;
    bool isPass() const override { return lostHP == 0; }
    json to_json(nlohmann::basic_json<> &j, const Step &step) override;

    static /*constexpr*/ ActionStep pass() { return { ActionCard::HARDATK, position(), position(), 0, 0 }; }
};

#endif //REVELATION_STEP_HPP
