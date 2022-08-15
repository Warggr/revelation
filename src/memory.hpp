#ifndef REVELATION_MEMORY_HPP
#define REVELATION_MEMORY_HPP

#include <memory>

template<typename T>
struct NullableShared {
private:
    std::unique_ptr<T> content;
public:
    NullableShared() = default;
    NullableShared(nullptr_t){};
    NullableShared(NullableShared&& move): content(std::move(move.content)) {}
    NullableShared& operator=(NullableShared&& move) {
        content = std::move(move.content); return *this;
    }
    NullableShared(const NullableShared& copy){
        if(this != &copy){
            if(copy.content != nullptr) content = std::make_unique<T>(*copy.content);
            else content = nullptr;
        }
    }
    NullableShared& operator=(const NullableShared& copy) {
        if(this != &copy){
            if(copy.content != nullptr) content = std::make_unique<T>(*copy.content);
            else content = nullptr;
        }
        return *this;
    }
    template<class... Args>
    NullableShared(Args&&... args): content(std::make_unique<T>(args...)) {}
    ~NullableShared(){}
    T& operator*(){ return *content; }
    const T& operator*() const { return *content; }
    T* operator->(){ return content.operator->(); }
    const T* operator->() const { return content.operator->(); }
    T* pt() { return content.get(); }
    const T* pt() const { return content.get(); }

    NullableShared copy() const { return NullableShared(*this); }
};

#endif //REVELATION_MEMORY_HPP
