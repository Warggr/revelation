#include "launch_game.hpp"
#include "search/search.hpp"
#include "search/depthfirstsearch.hpp"
#include "search/loggers.hpp"
#include "nlohmann/json.hpp"
#include <utility>

using nlohmann::json;
using json_ptr = nlohmann::json_pointer<json>;

#include "grammar.hpp" //include as a header

struct Scope {
    const json* j; json_ptr ptr; void* data;
    Scope(const json& j, json_ptr ptr, void* data = nullptr): j(&j), ptr(std::move(ptr)), data(data) {}
    Scope at(const std::string& key) const { return { (*j)[key], ptr/key, data }; }
    Scope at(unsigned int key) const { return { (*j)[key], ptr/key, data }; }
};

AgentDescription parseAgents(const json& j){
    Scope scope(j, json_ptr());
    return parse_root(scope);
}

#define FAIL(text) throw AgentCreationException(sc.ptr, text)
#define ASSERT(assertion, text) if(not (assertion)) FAIL(text)

#define START_SCOPE(new_scope) { const auto outer_scope = sc; sc = new_scope;
#define END_SCOPE sc = outer_scope; }

#undef CODE
#define CODE(x) x;
#undef SYMBOL
#define SYMBOL(symbol, c_type, body) c_type parse_##symbol(const Scope& root_scope) { auto sc = root_scope; c_type retVal; body return retVal; }

#define AS_ARRAY(arrsize, foreach) ASSERT(sc.j->is_array(), "Expected an array"); \
    ASSERT(sc.j->size() == (arrsize), "Expected " #arrsize " elements");               \
    for(uint i = 0; i < (arrsize); i++){ START_SCOPE(sc.at(i)) foreach END_SCOPE }

#define OPTION(option, body) if(optionstring == option) { body }

#define CHILD(key, body) ASSERT(sc.j->contains(key), "Missing key " key); START_SCOPE( sc.at(key) ); body END_SCOPE

#define GETSYMBOL(symbol, variable) auto variable = parse_##symbol(sc);
#define GETLITERAL(literal, variable) const std::string variable = #literal;

#define LITERAL(x) ASSERT(*sc.j == (x), "Expected literal" x);

#define SELECT_BY_TYPE(...) ASSERT(sc.j->contains("type"), "Missing key 'type'"); \
        std::string optionstring = (*sc.j)["type"].get<std::string>();           \
        MAKE_PAIRS(OPTION, else, __VA_ARGS__) \
        else { sc = sc.at("type"); FAIL("Unrecognized type"); }

#define SELECT_LITERAL(...) ASSERT(sc.j->is_string(), "String expected"); \
        std::string optionstring = (*sc.j).get<std::string>(); \
        MAKE_PAIRS(OPTION, else, __VA_ARGS__) \
        else FAIL("Unrecognized value");

#define OBJECT(...) ASSERT(sc.j->is_structured(), "Expected a structured object"); \
    MAKE_PAIRS(CHILD, , __VA_ARGS__)

#define HAS_TYPE(id) return nullptr;

#include "grammar.hpp" // include as function bodies
