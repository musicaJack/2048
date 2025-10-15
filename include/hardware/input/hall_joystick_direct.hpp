#ifndef _HALL_JOYSTICK_DIRECT_H_
#define _HALL_JOYSTICK_DIRECT_H_

#include <hardware/adc.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "hardware/system/pin_config.hpp"
#include "hardware/input/joystick_types.hpp"

/**
 * @brief 直接连接霍尔摇杆控制类
 * @description 使用RP2040内置ADC直接读取霍尔摇杆的模拟信号
 * 硬件连接：
 * - X轴: GP26 (ADC0)
 * - Y轴: GP27 (ADC1) 
 * - 开关: GP22 (数字输入，内部上拉)
 */
class HallJoystickDirect {
public:
    /**
     * @brief 摇杆初始化
     * @return true 成功, false 失败
     */
    bool begin();

    // 实现与 Joystick 类相同的接口
    uint16_t get_joy_adc_value_x(adc_mode_t adc_bits);
    uint16_t get_joy_adc_value_y(adc_mode_t adc_bits);
    uint8_t get_button_value(void);
    int16_t get_joy_adc_12bits_offset_value_x(void);
    int16_t get_joy_adc_12bits_offset_value_y(void);
    void get_joy_adc_16bits_value_xy(uint16_t *adc_x, uint16_t *adc_y);
    void get_joy_adc_8bits_value_xy(uint8_t *adc_x, uint8_t *adc_y);
    
    // LED控制方法
    void set_led(bool state);
    void update_led_from_joystick();

private:
    uint16_t _center_x;
    uint16_t _center_y;
    bool _calibrated;
    
    // TMR高频滤波器
    uint16_t _filter_x[4];  // 减少滤波器长度，提高响应速度
    uint16_t _filter_y[4];
    uint8_t _filter_index;
    bool _filter_initialized;
    
    // 滞回控制
    int16_t _last_stable_x;
    int16_t _last_stable_y;
    bool _hysteresis_initialized;

    // 辅助函数
    void calibrate_center(void);
    uint16_t read_adc_channel(uint channel);
    uint16_t apply_filter(uint16_t new_value, uint16_t* filter_buffer);
};

#endif
