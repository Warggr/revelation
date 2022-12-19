#ifndef REVELATION_WWW_VISITOR_HPP
#define REVELATION_WWW_VISITOR_HPP

#include "visitor.hpp"
#include <string>
#include <list>
#include <utility>
#include <algorithm>

class WwwDataVisitor : public WriterVisitor {
    const std::string _data;
    using KeyValueStore = std::list<std::pair<std::string_view, std::string_view>>;
    KeyValueStore parsed_data;
public:
    WwwDataVisitor(std::string data);

#define SUPPORTS(type) error_type visit(const std::string_view& key, type& value) override;
    SUPPORTS(std::string)
    SUPPORTS(bool)
    SUPPORTS(short)
    SUPPORTS(unsigned char)
    SUPPORTS(unsigned int)
    SUPPORTS(float)
    [[nodiscard]] bool empty() const override { return parsed_data.empty(); }
    [[nodiscard]] std::string_view anyKey() const override { return parsed_data.front().first; }
};

#endif //REVELATION_WWW_VISITOR_HPP
