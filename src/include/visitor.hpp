#ifndef REVELATION_VISITOR_HPP
#define REVELATION_VISITOR_HPP

#include <string_view>
#include <stdexcept>

class WriterVisitor {
public:
    enum error_type : bool { found = true, not_found = false };

    virtual ~WriterVisitor() = default;
#define SUPPORTS(type) virtual error_type visit(const std::string_view& key, type& value) = 0
    SUPPORTS(std::string);
    SUPPORTS(bool);
    SUPPORTS(short);
    SUPPORTS(unsigned char);
    SUPPORTS(float);
#undef SUPPORTS
    template<typename T>
    inline T get(const std::string_view& key, const T* defaultValue = nullptr){
        T retVal;
        if(not visit(key, retVal)) {
            if(defaultValue) return *defaultValue;
            else throw std::invalid_argument(std::string("Missing key:").append(key));
        }
        return retVal;
    }
};

#endif //REVELATION_VISITOR_HPP
