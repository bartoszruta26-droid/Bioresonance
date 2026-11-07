#include "timer1_pwm_calculator.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main() {
    timer1_pwm::Config c = {};

    assert(!timer1_pwm::calculate(16000000UL, 0, 50, &c));
    assert(!timer1_pwm::calculate(16000000UL, 100, 0, &c));
    assert(timer1_pwm::calculate(16000000UL, 100, 100, &c));
    assert(!timer1_pwm::calculate(16000000UL, 10, 50, &c)); // below timer range
    assert(timer1_pwm::calculate(16000000UL, 50000001UL, 50, &c)); // calculator can represent above API ceiling

    assert(timer1_pwm::calculate(16000000UL, 3800000UL, 50, &c)); // 38 kHz
    assert(c.prescaler == 1);
    assert(c.top == 420);
    assert(c.compare == 211);
    assert(c.actual_frequency_hz_x100 >= 3800000UL);
    assert(c.actual_frequency_hz_x100 <= 3801000UL);
    assert(timer1_pwm::calculate(16000000UL, 3800000UL, 100, &c));

    assert(timer1_pwm::calculate(16000000UL, 10000UL, 50, &c)); // 100 Hz
    assert(c.actual_frequency_hz_x100 > 0);
    assert(c.top <= 65535U);
    assert(c.compare > 0 && c.compare <= c.top);

    assert(timer1_pwm::calculate(16000000UL, 50000000UL, 50, &c)); // 500 kHz
    assert(c.actual_frequency_hz_x100 == 50000000UL);
    assert(c.prescaler == 1);
    assert(c.top == 31);

    assert(timer1_pwm::calculate(16000000UL, 24UL, 50, &c)); // 0.24 Hz
    assert(c.prescaler == 1024);
    assert(c.top <= 65535U);
    assert(c.actual_frequency_hz_x100 > 0);

    puts("Timer1 PWM calculator tests passed");
    return 0;
}
