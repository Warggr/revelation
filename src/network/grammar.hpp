/**
 * This file contains the definition of valid JSON accepted by the server to create agents.
 * Depending on how it is included, it can do different things:
 * - By default, it is a normal C/C++ header containing the definitions of the recognizer functions.
 * - With suitable macro definitions (see grammar.cpp), it generates the body of the recognizer functions as well.
 * - When PRODUCE_JSON is defined, and with suitable macro definitions, it expands to valid JSON
 *    defining the accepted grammar.
 *
 * The missing include guards are intentional!
 */

// Generally useful macros for all implementations
#define STR(x) #x
#define PARENS ()
#define ONE_EXPAND(...) __VA_ARGS__
#define THREE_EXPAND(...) ONE_EXPAND(ONE_EXPAND(ONE_EXPAND(__VA_ARGS__)))
#define EXPAND(...) THREE_EXPAND(THREE_EXPAND(THREE_EXPAND(__VA_ARGS__)))
// Preprocessor-related dark magic
#define NUM(...) SELECT_10TH(__VA_ARGS__, /*9*/ MANY, MANY, MANY, MANY, /*5*/MANY, MANY, MANY, TWO, ONE, throwaway)
#define SELECT_10TH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...) a10
#define PAIRS_MANY(pair_function, separator, a, b, ...) pair_function(a, b) separator PAIRS_AGAIN PARENS ( pair_function, separator, __VA_ARGS__ )
#define PAIRS_TWO(pair_function, s, a, b) pair_function(a, b)
#define PAIRS_HELPER(pf, s, qty, ...) PAIRS_##qty(pf, s, __VA_ARGS__)
#define XPAIRS_HELPER(pf, s, qty, ...) PAIRS_HELPER(pf, s, qty, __VA_ARGS__)
//Allow recursion
#define PAIRS_ONCE(pf, s, ...) XPAIRS_HELPER(pf, s, NUM(__VA_ARGS__), __VA_ARGS__)
#define PAIRS_AGAIN() PAIRS_ONCE
// MAKE_PAIRS(pair, sep, a1, b1, a2, b2, ..., a_n, b_n)
// will expand to:
// pair(a1, b1) sep pair(a2, b2) sep ... sep pair(a_n, b_n)
#define MAKE_PAIRS(pair_function, separator, ...) EXPAND(PAIRS_ONCE(pair_function, separator, __VA_ARGS__))

#ifdef PRODUCE_JSON
FIN({)
#else
#include "launch_game.hpp"
#include <memory>
struct Scope;
#ifndef SYMBOL
#define SYMBOL(symbol, c_type, body) c_type parse_##symbol(const Scope& root_scope);
#define CODE(x)
#endif
#endif

SYMBOL(staticDFS, std::unique_ptr<StaticDFS>,
    OBJECT(
        "type", LITERAL("staticDFS")
    )
    CODE(auto heuristic = reinterpret_cast<Heuristic*>(sc.data))
    CODE(retVal = std::make_unique<StaticDFS>(std::make_unique<NoOpLogger>(), *heuristic))
)

SYMBOL(botPolicy, std::unique_ptr<SearchPolicy>,
    GETSYMBOL(staticDFS, policy)
    CODE(retVal = std::move(policy))
)

SYMBOL(botHeuristic, std::unique_ptr<Heuristic>,
    LITERAL("pxt")
    CODE(retVal = std::make_unique<PowerTimesToughnessHeuristic>())
)

SYMBOL(onlineAgent, void*, HAS_TYPE("online"))
SYMBOL(localAgent, void*, HAS_TYPE("local"))
SYMBOL(randomAgent, void*, HAS_TYPE("random"))

CODE(unsigned int myId)

SYMBOL(botAgent, std::unique_ptr<SearchAgent>,
    CODE(std::unique_ptr<Heuristic> heuristic)
    CODE(std::unique_ptr<SearchPolicy> policy)
    OBJECT(
        "type", LITERAL("bot"),
        "heuristic", GETSYMBOL(botHeuristic, heur) CODE(heuristic = std::move(heur)),
        "policy",
            CODE( sc.data = reinterpret_cast<void*>(heuristic.get()) )
            GETSYMBOL(botPolicy, pl)
            CODE( policy = std::move(pl) )
   )
   CODE(retVal = std::make_unique<SearchAgent>(myId, std::move(policy), std::move(heuristic)))
)

SYMBOL(agent, AgentDescriptor,
    SELECT_BY_TYPE(
        "online", GETSYMBOL(onlineAgent, _) CODE( (void)_; retVal.type = AgentDescriptor::NETWORK ),
        "random", GETSYMBOL(randomAgent, _) CODE( (void)_; retVal.type = AgentDescriptor::RANDOM ),
        "bot",
            GETSYMBOL(botAgent, agent_data)
            CODE(retVal.type = AgentDescriptor::BOT)
            CODE(retVal.data = reinterpret_cast<void*>(agent_data.release()))
    )
)

SYMBOL(root, AgentDescription,
    AS_ARRAY(NB_AGENTS,
         CODE(myId = i)
         GETSYMBOL(agent, value)
         CODE(retVal[i] = value)
    )
)

#ifdef PRODUCE_JSON
FIN("ignore":null)
FIN(})
#endif
