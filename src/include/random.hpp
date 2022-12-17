#ifndef REVELATION_SEED_HPP
#define REVELATION_SEED_HPP

#include <random>

using Generator = std::minstd_rand;
using GeneratorSeed = unsigned long;

GeneratorSeed getRandom();

#endif //REVELATION_SEED_HPP
