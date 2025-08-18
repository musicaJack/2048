/**
 * @file random_generator.cpp
 * @brief 随机数生成器实现
 */

#include "utils/random_generator.hpp"
#include <cstdlib>
#include <ctime>

namespace utils {

RandomGenerator::RandomGenerator() {
    // 使用当前时间作为种子
    srand(time(nullptr));
}

RandomGenerator::~RandomGenerator() = default;

int RandomGenerator::getRandomInt(int min, int max) {
    if (min >= max) return min;
    return min + (rand() % (max - min + 1));
}

float RandomGenerator::getRandomFloat(float min, float max) {
    if (min >= max) return min;
    float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return min + random * (max - min);
}

bool RandomGenerator::getRandomBool() {
    return (rand() % 2) == 0;
}

} // namespace utils 
