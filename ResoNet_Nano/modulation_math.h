#ifndef RESONET_MODULATION_MATH_H
#define RESONET_MODULATION_MATH_H
#include <stdint.h>

namespace modulation_math {

// Integer parabolic approximation of a sine wave, scaled to [-1000, 1000].
// Avoids libm floating-point code on the ATmega328P.
static inline int16_t wave_x1000(uint16_t phase, uint16_t period) {
    if (period < 2U) return 0;
    phase = (uint16_t)(phase % period);
    const uint16_t half = (uint16_t)(period / 2U);
    const bool negative = phase >= half;
    const uint16_t x = negative ? (uint16_t)(phase - half) : phase;
    if (x >= half) return 0;

    const uint32_t denominator = (uint32_t)half * half;
    const uint32_t numerator =
        4000UL * (uint32_t)x * (uint32_t)(half - x);
    const int16_t magnitude =
        (int16_t)((numerator + denominator / 2UL) / denominator);
    return negative ? (int16_t)-magnitude : magnitude;
}

static inline uint8_t amplitude_duty(uint8_t base_percent,
                                     uint16_t phase, uint16_t period) {
    int16_t wave = wave_x1000(phase, period);
    uint16_t factor = (uint16_t)(500 + wave / 2);
    uint16_t duty = ((uint16_t)base_percent * factor + 500U) / 1000U;
    return duty > 100U ? 100U : (uint8_t)duty;
}

// Frequency deviation is +/-10%. Split the multiply to avoid 64-bit math.
static inline uint32_t frequency_hz_x100(uint32_t base_hz_x100,
                                         uint16_t phase, uint16_t period) {
    const int32_t wave = wave_x1000(phase, period);
    const int32_t high = (int32_t)(base_hz_x100 / 1000UL);
    const int32_t low = (int32_t)(base_hz_x100 % 1000UL);
    const int32_t delta = (high * wave) / 10L + (low * wave) / 10000L;
    const int32_t result = (int32_t)base_hz_x100 + delta;
    return result > 0 ? (uint32_t)result : 1UL;
}

} // namespace modulation_math
#endif
