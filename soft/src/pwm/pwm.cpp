/*
 * File: pwm.cpp
 * Description: PWM sys class interface
 * Created on: 25 February 2024
 */

#include "pwm.h"

namespace pwm
{
    // Pwm class
    int Pwm::num_instance = 0;

    Pwm::Pwm(int gpio, int period, int duty_cycle):
        gpio_(gpio),
        period_(period),
        duty_cycle_(duty_cycle)
    {
        num_instance++;
    }
}
