#pragma once

namespace utils {

/**
 * @brief 随机数生成器类
 * 
 * 提供各种类型的随机数生成功能
 */
class RandomGenerator {
public:
    RandomGenerator();
    ~RandomGenerator();

    // 禁用拷贝构造和赋值
    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;

    // 随机数生成方法
    int getRandomInt(int min, int max);
    float getRandomFloat(float min, float max);
    bool getRandomBool();

private:
    // 可以添加随机数种子等私有成员
};

} // namespace utils 
