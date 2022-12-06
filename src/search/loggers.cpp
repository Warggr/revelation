#include "loggers.hpp"
#include <iostream>

void SimpleIndentLogger::message( const char* msg ) const {
    for(unsigned i = 0; i < indent; i++) std::cout << "    ";
    ProgressLogger::message(msg);
}
void SimpleIndentLogger::message( const char* msg, float x ) const {
    for(unsigned i = 0; i < indent; i++) std::cout << "    ";
    ProgressLogger::message(msg, x);
}

void ProgressBar::enter(Timestep timestep, unsigned nbChildren) {
    assert(nbChildren != 0);
    if(timestep == Timestep::DISCARDED){
        assert(progress.empty());
        printf("[%4d\\   1]->", nbChildren);
        //std::cout << to_string(timestep) << ' ' << progress.size() << '\n';
        progress.push_back(0);
    } else if(timestep == Timestep::MOVEDlast){
        const unsigned int PROGRESS_LAST_INDEX = 0;
        assert(progress.size() == PROGRESS_LAST_INDEX + 1);
        progress[PROGRESS_LAST_INDEX] += 1;
        for(int i =0; i<7; i++) printf("\b");
        printf("%4d]->", progress[PROGRESS_LAST_INDEX]);
    } else if(timestep == Timestep::ABILITYCHOSEN){
        assert(progress.size() == 1);
        //std::cout << to_string(timestep) << ' ' << progress.size() << '\n';
        printf("<%4d\\   1>->", nbChildren);
        progress.push_back(0);
    } else if(timestep == Timestep::ACTED){
        const unsigned int PROGRESS_LAST_INDEX = 1;
        assert(progress.size() == PROGRESS_LAST_INDEX+1);
        progress[PROGRESS_LAST_INDEX] += 1;
        for(int i =0; i<7; i++) printf("\b");
        printf("%4d>->", progress[PROGRESS_LAST_INDEX]);
    }
}

void ProgressBar::exit(Timestep timestep) {
    if(timestep == Timestep::DISCARDED or timestep == Timestep::ABILITYCHOSEN){
        for(uint i=0; i<13; i++) printf("\b"); // deleting my progress-frame
        //std::cout << "Exiting\n";
        progress.pop_back();
    }
}

void ProgressBar::enterTurn() {
    //std::cout << "Entering turn\n";
    progressStack.push_back(std::move(progress));
    progress = std::vector<int>();
}

void ProgressBar::exitTurn() {
    //std::cout << "Exiting turn\n";
    for(uint i = 0; i<13 * progress.size(); i++) printf("\b"); // deleting progress-frames for all states in the turn which were not exited yet
    fflush(stdout);
    progress = std::move(progressStack.back());
    progressStack.pop_back();
}

void ProgressLogger::message(const char* msg) const {
    std::cout << msg << '\n';
}

void ProgressLogger::message(const char* msg, float nb) const {
    std::cout << msg << ' ' << nb << '\n';
}

void ProgressLogger::enter(Timestep timestep, unsigned nbChildren){
    message(std::string("ENTER ").append(to_string(timestep)).c_str(), nbChildren);
}

void ProgressLogger::exit(Timestep timestep){
    message(std::string("EXIT ").append(to_string(timestep)).c_str());
}
