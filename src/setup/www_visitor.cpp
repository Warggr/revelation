#include "www_visitor.hpp"

std::string urldecode(std::string_view encoded){
    std::string retVal(encoded);
    std::string::size_type pos = 0;
    while(true){
        pos = retVal.find('+', pos);
        if(pos == retVal.npos) break;
        retVal.replace(pos, 1, " ");
        pos += 1;
    }
    return retVal;
}

WwwDataVisitor::WwwDataVisitor(std::string data) : _data(std::move(data)){
    for(uint cursor = 0; cursor != _data.size() + 1; ){
        auto posEqual = _data.find('=', cursor);
        if(posEqual == std::string_view::npos) {
            std::string error = "Missing a '=' here:\n";
            error += _data; error += '\n';
            for(uint i = 0; i<cursor; i++) error += ' ';
            error += '^';
            throw std::invalid_argument(error);
        }
        std::string_view key( _data.data() + cursor, posEqual - cursor);
        cursor = posEqual + 1;
        auto posAnd = _data.find('&', posEqual+1);
        if(posAnd == std::string_view::npos)
            posAnd = _data.size();
        std::string_view value( _data.data() + cursor, posAnd - cursor);
        cursor = posAnd + 1;
        parsed_data.emplace_back(std::make_pair( key, value ));
    }
}

WriterVisitor::error_type WwwDataVisitor::visit(const std::string_view& key, std::string& value) {
    auto iter = std::find_if(parsed_data.begin(), parsed_data.end(),
                             [&key](const std::pair<std::string_view, std::string_view> iter){ return iter.first == key; });
    if(iter == parsed_data.end()) return not_found;
    value = urldecode(iter->second);
    parsed_data.erase(iter);
    return found;
}

WriterVisitor::error_type WwwDataVisitor::visit(const std::string_view& key, bool& value) {
    std::string rawValue;
    auto result = visit(key, rawValue);
    value = result; // HTML checkboxes: no value provided when false, value provided when true
    return found;
}

WriterVisitor::error_type WwwDataVisitor::visit(const std::string_view& key, short& value) {
    std::string rawValue; auto result = visit(key, rawValue); if(result == not_found) return not_found;
    value = std::stoi(rawValue); // I know, this is inefficient and overflow-unsafe
    return found;
}

WriterVisitor::error_type WwwDataVisitor::visit(const std::string_view& key, unsigned int& value) {
    std::string rawValue; auto result = visit(key, rawValue); if(result == not_found) return not_found;
    value = std::stoi(rawValue);
    return found;
}

WriterVisitor::error_type WwwDataVisitor::visit(const std::string_view& key, unsigned char& value) {
    std::string rawValue; auto result = visit(key, rawValue); if(result == not_found) return not_found;
    value = std::stoi(rawValue); // I know, this is inefficient and overflow-unsafe
    return found;
}

WriterVisitor::error_type WwwDataVisitor::visit(const std::string_view& key, float& value) {
    std::string rawValue; auto result = visit(key, rawValue); if(result == not_found) return not_found;
    value = std::stof(rawValue); // I know, this is inefficient and overflow-unsafe
    return found;
}
