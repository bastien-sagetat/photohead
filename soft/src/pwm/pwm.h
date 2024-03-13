/*
 * File: pwm.h
 * Description: PWM sys class interface
 * Created on: 25 February 2024
 */

#ifndef PWM_H_
#define PWM_H_


namespace pwm
{
    /**
     * \class PWM
     * Drives hardware PWM.
     */
    class Pwm
    {
    public:
        Pwm(int gpio, int period, int duty_cycle);


    private:
        static int num_instance;
        int gpio_;       // GPIO pin ID
        int period_;     // Period in nanoseconds
        int duty_cycle_; // Duty cycle in nanoseconds
    };

}

#endif // !defined(OBJECT_DETECTION_H_)
