#include "file_logger.hpp"

FileLogger::FileLogger(std::ostream& file, std::string_view start) : file(file) {
    file << "{\"state\":" << start << ",\"steps\":[";
    firstStep = true;
}

FileLogger::~FileLogger() {
    file << "]}";
}

void FileLogger::addStep(std::string_view step) {
    if(firstStep) firstStep = false;
    else file << ',';
    file << step << '\n';
}
