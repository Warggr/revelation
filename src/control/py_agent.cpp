#include "py_agent.hpp"
#include "gameplay/state.hpp"
#include "setup/units_repository.hpp"
extern "C" {
    #include "Python.h"
}

// Adapted from https://gist.github.com/octavifs/5362297.
// Converts a C++ map to a python dict
template <class K, class V>
boost::python::dict toPythonDict(const std::unordered_map<K, V>& map) {
	boost::python::dict dictionary;
	for (const auto& iter : map) {
		dictionary[iter.first] = iter.second;
	}
	return dictionary;
}

template<typename T>
struct Wrapper {
    T data;
    Wrapper(T data): data(data) {};
    const T get() const { return data; }
};

bpy::dict UnitsRepository_getTeams(const Wrapper<const UnitsRepository&> repo){
    return toPythonDict(repo.get().getTeams());
}

BOOST_PYTHON_MODULE(setup){
    bpy::class_<Team>("Team")
    ;

    bpy::class_<Wrapper<const UnitsRepository&>>("UnitsRepository", bpy::no_init)
        .add_property("teams", &UnitsRepository_getTeams)
    ;
}

template<typename ReturnType, typename... Args>
ReturnType PyStrategy::call_python_method(const char* funcName, Args&&... args){
    try {
        auto func = agent.attr(funcName);
        auto result_obj = func(std::forward<Args>(args)...);
        auto result = bpy::extract<ReturnType>(result_obj);
        return result;
    } catch( const bpy::error_already_set& err ) {
        PyErr_Print();
        throw err;
    }
}

PyStrategy::PyStrategy(std::string_view filename) {
    try {
        setenv("PYTHONPATH", ".", 1); // Allow loading modules from current directory
        PyImport_AppendInittab("setup", &PyInit_setup); //register statically linked modules
        Py_Initialize();

        bpy::object module = bpy::import(bpy::str(std::string(filename)));
        bpy::object strategy_class = module.attr("Strategy");
        agent = strategy_class();
    } catch(const bpy::error_already_set& err ) {
        PyErr_Print();
        throw err;
    }
}

const Team& PyAgent::getTeam(const UnitsRepository& repo) {
    return strategy.call_python_method<const Team&>("getTeam", Wrapper<const UnitsRepository&>(repo));
}

ActionDecision PyAgent::getAction(const State& state) {
    return strategy.call_python_method<ActionDecision>("getAction", state);
}

DiscardDecision PyAgent::getDiscard(const State& state) {
    return strategy.call_python_method<DiscardDecision>("getDiscard", state);
}

MoveDecision PyAgent::getMovement(const State& state, unsigned int nb) {
    return strategy.call_python_method<MoveDecision>("getMovement", state, nb);
}

unsigned int PyAgent::getSpecialAction(const State&, const Effect&) {
    return 0;
}

uint SimplePyAgent::choose(const OptionList& options, const std::string_view& message) {
    bpy::list py_list;
    for (const auto& option: options) {
        py_list.append(bpy::make_tuple(std::string(option.first),std::string(option.second)));
    }
    int result = strategy.call_python_method<int>("choose", py_list, std::string(message));
    return static_cast<uint>(result);
}
