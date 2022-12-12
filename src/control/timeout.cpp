#include "timeout.hpp"
#include "gameplay/state.hpp"
#include "setup/units_repository.hpp"
#include <thread>
#include <future>
#include <cassert>

template<typename RetType, typename... Args>
RetType AgentTimeoutProxy::timeout(RetType (Agent::*function)(Args...), Args... args){
    using namespace std::chrono_literals;
    std::future<RetType> result_promise = std::async(
            std::launch::async,
            [agent=agent.get(),function,&args...]() -> RetType {
                return (agent->*function)(args...);
            });
    std::future_status success = result_promise.wait_for(3s);
    if(success == std::future_status::ready) return result_promise.get();
#ifndef NDEBUG
    else if(success == std::future_status::deferred) assert(false);
#endif
    else {
        agent->interrupt();
        Agent& fallback = agent->getFallback();
        return (fallback.*function)(args...);
    }
}

#define PASS_WITH_TIMEOUT_1_ARGS(funcName, retType, argType) \
    retType AgentTimeoutProxy::funcName(argType arg) { return timeout<retType, argType>(&Agent::funcName, arg); }
#define PASS_WITH_TIMEOUT_2_ARGS(funcName, retType, argType1, argType2) \
    retType AgentTimeoutProxy::funcName(argType1 arg1, argType2 arg2) { return timeout<retType, argType1, argType2>(&Agent::funcName, arg1, arg2); }

PASS_WITH_TIMEOUT_1_ARGS(onBegin, void, const State&)

PASS_WITH_TIMEOUT_1_ARGS(getTeam, const Team&, const UnitsRepository&)

PASS_WITH_TIMEOUT_1_ARGS(getDiscard, DiscardDecision, const State&)

PASS_WITH_TIMEOUT_1_ARGS(getAction, ActionDecision, const State&)

PASS_WITH_TIMEOUT_2_ARGS(getMovement, MoveDecision, const State&, unsigned)

PASS_WITH_TIMEOUT_2_ARGS(getSpecialAction, unsigned int, const State&, const Effect&)
