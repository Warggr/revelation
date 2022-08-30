#ifndef REVELATION_PROGRESSLOGGERS_HPP
#define REVELATION_PROGRESSLOGGERS_HPP

#include "depthfirstsearch.hpp"

class SimpleIndentLogger: public ProgressLogger{
    unsigned short int indent = 0;
public:
    void enterTurn() override { indent += 1; }
    void exitTurn() override { indent -= 1; }
    void message( const char* msg ) const override;
    void message( const char* msg, float x ) const override;
};

class ProgressBar : public ProgressLogger{
    std::vector<std::vector<int>> progressStack;
    std::vector<int> progress;
public:
    void message( const char* ) const override {};
    void message( const char*, float ) const override {};
    void enter(Timestep timestep, unsigned nbChildren) override;
    void exit(Timestep timestep) override;
    void enterTurn() override;
    void exitTurn() override;
};

class NoOpLogger : public ProgressLogger {
public:
    void enterTurn() override {};
    void exitTurn() override {};
    void enter(Timestep, unsigned int) override {};
    void exit(Timestep) override {};
    void message(const char*) const override {};
    void message(const char*, float) const override {};
};

#endif //REVELATION_PROGRESSLOGGERS_HPP
