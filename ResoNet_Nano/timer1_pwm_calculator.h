#ifndef TIMER1_PWM_CALCULATOR_H
#define TIMER1_PWM_CALCULATOR_H

#include <stdint.h>

namespace timer1_pwm {

struct Config {
    uint16_t prescaler;
    uint16_t top;
    uint16_t compare;
    uint32_t actual_frequency_hz_x100;
};

inline bool calculate(uint32_t cpu_hz, uint32_t target_hz_x100,
                      uint8_t duty_percent, Config* out) {
    if (out == 0 || cpu_hz == 0 || target_hz_x100 == 0 ||
        duty_percent == 0 || duty_percent >= 100) {
        return false;
    }

    static const uint16_t prescalers[] = {1, 8, 64, 256, 1024};
    bool found = false;
    uint64_t best_error = UINT64_MAX;
    Config best = {0, 0, 0, 0};

    for (uint8_t i = 0; i < sizeof(prescalers) / sizeof(prescalers[0]); ++i) {
        const uint64_t denominator =
            static_cast<uint64_t>(prescalers[i]) * target_hz_x100;
        const uint64_t numerator = static_cast<uint64_t>(cpu_hz) * 100ULL;
        const uint64_t counts = (numerator + denominator / 2ULL) / denominator;

        // Fast PWM mode 14 requires at least two timer counts. TOP is counts - 1.
        if (counts < 2ULL || counts > 65536ULL) {
            continue;
        }

        const uint64_t actual =
            (numerator + denominator * counts / prescalers[i] / 2ULL) /
            (static_cast<uint64_t>(prescalers[i]) * counts);
        const uint64_t error = actual > target_hz_x100
            ? actual - target_hz_x100 : target_hz_x100 - actual;

        uint64_t compare =
            (counts * duty_percent + 50ULL) / 100ULL;
        if (compare < 1ULL) compare = 1ULL;
        if (compare >= counts) compare = counts - 1ULL;

        if (!found || error < best_error ||
            (error == best_error && counts > static_cast<uint64_t>(best.top) + 1ULL)) {
            best.prescaler = prescalers[i];
            best.top = static_cast<uint16_t>(counts - 1ULL);
            best.compare = static_cast<uint16_t>(compare);
            best.actual_frequency_hz_x100 =
                static_cast<uint32_t>((numerator +
                    static_cast<uint64_t>(prescalers[i]) * counts / 2ULL) /
                    (static_cast<uint64_t>(prescalers[i]) * counts));
            best_error = error;
            found = true;
        }
    }

    if (!found) return false;
    *out = best;
    return true;
}

} // namespace timer1_pwm

#endif
