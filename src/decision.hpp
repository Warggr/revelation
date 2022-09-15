#ifndef REVELATION_DECISION_HPP
#define REVELATION_DECISION_HPP

/** class Decision is like an interface, except that it has no methods at all.
 * We can just assume that every Decision can be dynamic_cast to one of the decision classes (ActionDecision, MoveDecision, etc.)
 * We could even not created this class at all and used a void* instead of Decision* in all methods.
 */
class Decision {
public:
    virtual ~Decision() = default;
    virtual bool isPass() const = 0;
};

struct DiscardDecision final : public Decision {
    bool discardedAction;
    unsigned int iCardDiscarded;
    DiscardDecision(bool discardedAction = true, unsigned int iCardDiscarded = 0):
        discardedAction(discardedAction), iCardDiscarded(iCardDiscarded) {};
    bool isPass() const override { return false; }
};

class ActionDecision final : public Decision {
public:
    ActionCard card;
    position subjectPos;
    position objectPos;

    ActionDecision() = default;
    constexpr ActionDecision(ActionCard card, position subjectPos, position objectPos)
            : card(card), subjectPos(subjectPos), objectPos(objectPos) {}

    /*constexpr*/ bool isPass() const override {
        return card==ActionCard::DEFENSE and subjectPos == position(1, 1) and objectPos == position(1, 1);
    }
    static /*constexpr*/ ActionDecision pass(){
        return { ActionCard::DEFENSE, {1, 1}, {1, 1} };
    }
};

// static_assert(ActionDecision::pass().isPass() );

struct MoveDecision final : public Decision {
    position from;
    position to;
    std::vector<Direction> moves;
    MoveDecision() = default;
    MoveDecision(position from, position to, std::vector<Direction> moves): from(from), to(to), moves(std::move(moves)) {};

    static MoveDecision pass() { return { {}, {}, {} }; };
    bool isPass() const override {
        return moves.empty();
    }
};

//static_assert( MoveDecision::pass().isPass() );

struct AbilityDecision final : public Decision {
    bool isPass() const override { return true; }
};

#endif //REVELATION_DECISION_HPP
