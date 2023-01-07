#include "setup/unique_character.hpp"
#include "nlohmann/json.hpp"

using nlohmann::json;

std::string to_string(const ImmutableCharacter& chr){
    return json(chr).dump();
}
