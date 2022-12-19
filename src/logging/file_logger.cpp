#include "file_logger.hpp"
#include <ostream>
#ifndef NDEBUG
#include <cassert>
#include <fstream>
#endif

FileLogger::FileLogger(std::ostream& file, std::string_view start) : file(file) {
    file << "{\"state\":" << start << ",\"steps\":[";
    firstStep = true;
}

FileLogger::~FileLogger() {
#ifndef NDEBUG
    auto fstream = dynamic_cast<std::ofstream*>(&file);
    if(fstream){
        assert(fstream->is_open());
    }
#endif
    file << "]}";
    file.flush();
}

void FileLogger::addStep(std::string_view step) {
    if(firstStep) firstStep = false;
    else file << ',';
    file << step << '\n';
}
