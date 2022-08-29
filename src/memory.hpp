#ifndef REVELATION_MEMORY_HPP
#define REVELATION_MEMORY_HPP

#include <memory>

/**
 * This class behaves exactly like a smart pointer / std::unique_ptr, except for a few differences:
 * - NullableShared's are created via `auto x = NullableShared(...)` instead of `auto x = std::make_unique(...)`
 * - The method NullableShared::copy creates a copy of the pointed object and a new NullableShared owning that copy.
 *
 * Currently, the class is rather useless.
 * The goal is that in the future, the copy() method returns a shallow copy pointing to the same object,
 * and that the object is only copied when it is updated (copy-on-write)
 */
template<typename T>
struct NullableShared {
private:
    std::unique_ptr<T> content;
public:
    NullableShared() = default;
    NullableShared(std::nullptr_t){};
    NullableShared(NullableShared&& move): content(std::move(move.content)) {}
    NullableShared& operator=(NullableShared&& move) noexcept {
        content = std::move(move.content); return *this;
    }
    template<class... Args>
    NullableShared(Args&&... args): content(std::make_unique<T>(args...)) {}

    T& operator*(){ return *content; }
    const T& operator*() const { return *content; }
    T* operator->(){ return content.operator->(); }
    const T* operator->() const { return content.operator->(); }
    T* get() { return content.get(); }
    const T* get() const { return content.get(); }

    explicit operator bool() const noexcept { return static_cast<bool>(content); }

    NullableShared copy() const {
        NullableShared retVal(nullptr);
        if(content != nullptr) retVal.content = std::make_unique<T>(*content);
        return retVal;
    }
};

#endif //REVELATION_MEMORY_HPP
