#ifndef REVELATION_FILE_LOGGER_HPP
#define REVELATION_FILE_LOGGER_HPP

#include "logger.hpp"
#include <ostream>

class FileLogger : public SubLogger {
    std::ostream& file;
    bool firstStep;
public:
    FileLogger(std::ostream& file, std::string_view start);
    ~FileLogger() override;
    void addStep(std::string_view step) override;
};

#endif //REVELATION_FILE_LOGGER_HPP
