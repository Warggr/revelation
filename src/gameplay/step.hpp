//
// Created by Diana Amirova on 08.07.22.
//

#ifndef REVELATION_STEP_HPP
#define REVELATION_STEP_HPP

#include "nlohmann/json_fwd.hpp"
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

#endif //REVELATION_STEP_HPP
