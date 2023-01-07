#include "control/game.hpp"
#include "search/search.hpp"
#include "search/depthfirstsearch.hpp"
#include "search/loggers.hpp"
#include "network/server_impl.hpp"
#include "setup/units_repository.hpp"
#include "setup/www_visitor.hpp"
#include "random.hpp"

#ifndef NDEBUG
#define SOL_CHECK_ARGUMENTS
#define SOL_ALL_SAFETY_ON
#endif
#include "sol/sol.hpp"

#include <lua.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <memory>
#include <cassert>

[[maybe_unused]] static void dumpstack (lua_State *L) {
    int top=lua_gettop(L);
    for (int i=1; i <= top; i++) {
        printf("%d\t%s\t", i, luaL_typename(L,i));
        switch (lua_type(L, i)) {
        case LUA_TNUMBER:
            printf("%g\n",lua_tonumber(L,i));
            break;
        case LUA_TSTRING:
            printf("%s\n",lua_tostring(L,i));
            break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            printf("%s\n", "nil");
            break;
        default:
            printf("%p\n",lua_topointer(L,i));
            break;
        }
    }
}

template<typename T>
struct MallocUniquePtr {
    T* data;
    MallocUniquePtr(T* data = nullptr): data(data){};
    ~MallocUniquePtr(){ free(data); }
    T* get() const noexcept { return data; }
    operator bool() const noexcept { return data != nullptr; }
};

extern "C" int print(lua_State* state){
    const char* message = lua_tostring(state, -1);
    std::cout << "MSG:" << message << '\n';
    lua_pop(state, 1);
    return 0;
}

template<typename T>
struct Wrapper {
    T data;
    Wrapper(T data): data(data) {};
    const T get() const { return data; }
};

extern "C" int luaopen_lua(lua_State* state){
    std::cout << "onload\n";
    // Creating a wrapper around the state
    sol::state_view lua(state);
    lua_register(state, "log", print);

    auto generator_type = lua.new_usertype<Generator>(
        "Generator",
        sol::factories(
            [](int seed) -> Generator { return Generator(seed); },
            []() -> Generator { return Generator(getRandom()); }
        )
    );
    generator_type["__call"] = [](Generator& gen){ return gen(); };
    generator_type["__tostring"] = [](){ return "Generator"; };

    auto characterref_type = lua.new_usertype<Wrapper<CharacterRef>>(
        "Character", sol::no_constructor
    );
    characterref_type["__tostring"] = [](Wrapper<CharacterRef>& w){
        return to_string(*w.data);
    };

    auto teamref_type = lua.new_usertype<Wrapper<const Team*>>(
        "Team", sol::no_constructor
    );
    teamref_type["name"] = sol::property(
        [](Wrapper<const Team*> w) -> std::string { return w.get()->name; }
    );
    teamref_type["getUnits"] = [](const Wrapper<const Team*>& team) -> std::vector<Wrapper<CharacterRef>> {
        std::vector<Wrapper<CharacterRef>> retVal;
        for(const auto& row : team.get()->characters)
            for(const auto& unit : row)
                retVal.emplace_back(unit);
        return retVal;
    };

    auto repo_type = lua.new_usertype<UnitsRepository>(
        "UnitsRepository",
        sol::constructors<UnitsRepository()>()
    );
    repo_type["getRandom"] = [](UnitsRepository& repo, Generator& gen) -> Wrapper<CharacterRef> {
        return { repo.getRandomUnit(gen) };
    };
    repo_type["parse"] = [](UnitsRepository& repo, const std::string& str){
        std::stringstream ss(str);
        repo.readFile(ss);
    };
    repo_type["getCharacters"] = [](UnitsRepository& repo) -> std::unordered_map<CharacterId, Wrapper<CharacterRef>> {
        std::unordered_map<CharacterId, Wrapper<CharacterRef>> retVal;
        for(const auto& [key, value] : repo.getCharacters()){
            auto [iter, success] = retVal.emplace( key, &value );
            assert(success);
        }
        return retVal;
    };
    repo_type["getTeams"] = [](UnitsRepository& repo) -> std::unordered_map<TeamId, Wrapper<const Team*>> {
        std::unordered_map<TeamId, Wrapper<const Team*>> retVal;
        std::cout << "Getting teams:\n";
        for(const auto& [key, value] : repo.getTeams()){
            auto [iter, success] = retVal.emplace( key, &value );
            std::cout << "Created Wrapper<const Team*> to wrap address " << &value << '\n';
            assert(success);
        }
        return retVal;
    };

    auto randomUnitCreator_type = lua.new_usertype<UnitsRepository::NewRandomUnitProvider>(
        "RandomUnitCreator",
        sol::factories(
            [](sol::userdata rep) -> UnitsRepository::NewRandomUnitProvider {
                return { rep.as<UnitsRepository>() };
            }
        )
    );
    randomUnitCreator_type["getRandom"] = [](UnitsRepository::NewRandomUnitProvider& creator, Generator& gen) -> Wrapper<CharacterRef> {
        return { creator.getRandomUnit(gen) };
    };

    auto heuristic_type = lua.new_usertype<Heuristic>("Heuristic", sol::no_constructor);
    heuristic_type["pxt"] = []() -> std::unique_ptr<Heuristic> { return std::make_unique<PowerTimesToughnessHeuristic>(); };
    heuristic_type["death"] = []() -> std::unique_ptr<Heuristic> { return std::make_unique<SomeoneDiesHeuristic>(); };

    auto progresslogger_type = lua.new_usertype<ProgressLogger>("ProgressLogger", sol::no_constructor);
    progresslogger_type["noop"] = []() -> std::shared_ptr<ProgressLogger> { return std::make_shared<NoOpLogger>(); };

    auto searchpolicy_type = lua.new_usertype<SearchPolicy>("SearchPolicy", sol::no_constructor);
    searchpolicy_type["staticDFS"] = [](std::shared_ptr<ProgressLogger> logger, const std::unique_ptr<Heuristic>& heuristic) -> std::unique_ptr<SearchPolicy> {
        return std::make_unique<StaticDFS>(logger, *heuristic.get());
    };

    auto agent_type = lua.new_usertype<Agent>("Agent", sol::no_constructor);
    agent_type["search"] = [](int myId, uptr<SearchPolicy>& policy, uptr<Heuristic>& heuristic) -> std::unique_ptr<Agent> {
        return std::make_unique<SearchAgent>( myId, std::move(policy), std::move(heuristic) ); 
    };

    auto gamesummary_type = lua.new_usertype<GameSummary>("GameSummary", sol::no_constructor);
    gamesummary_type["whoWon"] = &GameSummary::whoWon;

    auto game_type = lua.new_usertype<Game>("Game", sol::factories(
        [](sol::this_state state, sol::table teams_lua, sol::table agents_lua, GeneratorSeed seed) -> Game {
            std::array<Agent*, NB_AGENTS> agents;
            std::array<const Team*, NB_AGENTS> teams;

            if(teams_lua.size() != 2) luaL_argerror(state, 0, "Expected array with 2 entries");
            for(uint i = 1; i<=NB_AGENTS; i++){
                auto team = teams_lua.get<sol::optional<Wrapper<const Team*>>>( i );
                if(not team) luaL_argerror(state, 0, (std::string("teams[") + std::to_string(i) + "] is not a Team").c_str() );
                else teams[i - 1] = team.value().get();
                std::cout << "Pop team with address " << teams[i - 1] << " for position " << i << '\n';
            }

            if(agents_lua.size() != 2) luaL_argerror(state, 1, "Expected array with 2 entries");
            for(const auto& [key_obj, value] : agents_lua){
                int key = key_obj.as<int>();
                if(key != 1 and key != 2) luaL_argerror(state, 1, "Unexpected key");
                if(not value.is<std::unique_ptr<Agent>>()) luaL_argerror(state, 1, (std::string("agents[") + std::to_string(key) + "] not an agent").c_str() );
                else agents[key - 1] = value.as<std::unique_ptr<Agent>&>().get();
            }
            return Game(teams, agents, seed);
        }
    ));

    return 1;
}
