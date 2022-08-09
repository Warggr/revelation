//
// Created by Diana Amirova on 08.07.22.
//

#ifndef REVELATION_STEP_HPP
#define REVELATION_STEP_HPP

#include "string"
#include "constants.hpp"
#include "deck.hpp"
#include "nlohmann/json_fwd.hpp"
#include "position.hpp"
#include "character.hpp"
#include <variant>
#include <memory>

using json = nlohmann::json;

template<typename T>
using uptr = std::unique_ptr<T>;

struct Step {
    virtual ~Step() = default;
    virtual void to_json(json& j) const = 0;
    virtual bool isPass() const = 0;
};

inline void to_json(json& j, const Step& step){ step.to_json(j); }

struct BeginStep : public Step{
    bool isPass() const override { return false; }
    void to_json(json& j) const override;
};

struct DrawStep : public Step {
    std::variant<ActionCard, Faction> cardDrawn;
    std::tuple<int, int> size;

    DrawStep(std::variant<ActionCard, Faction> cardDrawn, std::tuple<int, int> size){
        this->cardDrawn = cardDrawn;
        this->size = size;
    }
    bool isPass() const override { return false; }
    void to_json(json& j) const override;
};

struct MoveStep : public Step {
    position from;
    position to;
    char uid;
    std::vector<Direction> moves;
    int firstCOF;

    MoveStep(position from, position to, char uid, std::vector<Direction> moves, int firstCOF) : from(from), to(to), uid(uid) {
        this->moves = moves;
        this->firstCOF = firstCOF;
    }
    static MoveStep pass() { return { {}, {}, '0', {}, 0 }; }
    bool isPass() const override { return moves.empty(); }
    void to_json(json& j) const override;
};

struct AbilityStep : public Step {
    bool isPass() const override { return true; }
    void to_json(json& j) const override;
};

struct ActionStep final: public Step {
    ActionCard cardLost;
    position subject, object;
    unsigned int setHP;
    unsigned int lostHP;
    bool del = false;

    constexpr ActionStep(ActionCard cardLost, position subject, position object, unsigned int setHP, int diffHP):
        cardLost(cardLost), subject(subject), object(object), setHP(setHP), lostHP(diffHP) {};
    ~ActionStep() final = default;
    bool isPass() const override { return lostHP == 0; }
    void to_json(json& j) const override;

    static /*constexpr*/ ActionStep pass() { return { ActionCard::HARDATK, position(), position(), 0, 0 }; }
};

#endif //REVELATION_STEP_HPP
