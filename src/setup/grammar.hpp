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
#define COMMA ,
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
#include "agent_setup.hpp"
#include <memory>
struct Scope;
#ifndef SYMBOL
#define SYMBOL(symbol, c_type, body) c_type parse_##symbol(const Scope& root_scope);
#define CODE(x)
#endif
#endif

SYMBOL(staticDFS, std::unique_ptr<StaticDFS>,
    CODE(auto heuristic = reinterpret_cast<Heuristic*>(sc.data))
    CODE(retVal = std::make_unique<StaticDFS>(std::make_unique<NoOpLogger>(), *heuristic))
    CLASS(
        CLASS_REQUIRES(
            "type", LITERAL("staticDFS")
        ),
        CLASS_ALLOWS(
            "opponentsTurn", GETSYMBOL(botPolicy, auto oppTurn) CODE(retVal->setOpponentsTurn(std::move(oppTurn)))
        )
    )
)

SYMBOL(BFS, std::unique_ptr<GreedyBestFirstSearchAgent>,
    CODE(auto heuristic = reinterpret_cast<Heuristic*>(sc.data))
    CLASS(
        CLASS_REQUIRES(
            "type", LITERAL("BFS"),
            "depth", GETSYMBOL(integer, unsigned int maxDepth) CODE(retVal = std::make_unique<GreedyBestFirstSearchAgent>(*heuristic, maxDepth))
        ),
        CLASS_ALLOWS_NONE
    )
)

SYMBOL(botPolicy, std::unique_ptr<SearchPolicy>,
    SELECT_BY_TYPE(
        "staticDFS", staticDFS,
        "BFS", BFS
    )
)

SYMBOL(botHeuristic, std::unique_ptr<Heuristic>,
    LITERAL("pxt")
    CODE(retVal = std::make_unique<PowerTimesToughnessHeuristic>())
)

SYMBOL(onlineAgent, AgentDescriptor, HAS_TYPE("online") CODE(retVal.type = AgentDescriptor::NETWORK))
SYMBOL(randomAgent, AgentDescriptor, HAS_TYPE("random") CODE(retVal.type = AgentDescriptor::RANDOM))

CODE(unsigned int myId)

SYMBOL(botAgent, AgentDescriptor,
    CODE(std::unique_ptr<Heuristic> heuristic)
    CODE(std::unique_ptr<SearchPolicy> policy)
    OBJECT(
        "type", LITERAL("bot"),
        "heuristic", GETSYMBOL(botHeuristic, heuristic),
        "policy",
            CODE( sc.data = reinterpret_cast<void*>(heuristic.get()) )
            GETSYMBOL(botPolicy, policy)
    )
    CODE(retVal.data = reinterpret_cast<void*>(new SearchAgent(myId, std::move(policy), std::move(heuristic))))
    CODE(retVal.type = AgentDescriptor::BOT)
)

SYMBOL(timeoutProxy, AgentDescriptor,
    OBJECT(
        "type", LITERAL("timeoutProxy"),
        "proxy-for", GETSYMBOL(agent, auto inner_agent) CODE( retVal.data = reinterpret_cast<void*>( new AgentDescriptor(inner_agent) ) )
    )
    CODE(retVal.type = AgentDescriptor::TIMEOUT_PROXY)
)

SYMBOL(agent, AgentDescriptor,
    SELECT_BY_TYPE(
        "online", onlineAgent,
        "random", randomAgent,
        "bot", botAgent,
        "timeoutProxy", timeoutProxy
    )
)

SYMBOL(agents, AgentDescription,
    AS_ARRAY(NB_AGENTS,
        CODE(myId = i)
        GETSYMBOL(agent, retVal[i])
    )
)

SYMBOL(root, GameDescription,
    CLASS(
        CLASS_REQUIRES_NONE,
        CODE(retVal.agents = { AgentDescriptor::NETWORK COMMA AgentDescriptor::NETWORK })
        CLASS_ALLOWS(
            "agents", GETSYMBOL(agents, retVal.agents),
            "seed", GETSYMBOL(integer, retVal.seed),
            "teams", AS_ARRAY(NB_AGENTS,
                GETSYMBOL(string, retVal.teams[i])
            )
        )
    )
)

#ifdef PRODUCE_JSON
FIN("ignore":null)
FIN(})
#endif
