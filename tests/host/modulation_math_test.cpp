#include "modulation_math.h"
#include <assert.h>
#include <stdint.h>
int main() {
    assert(modulation_math::wave_x1000(0, 1000) == 0);
    assert(modulation_math::wave_x1000(250, 1000) == 1000);
    assert(modulation_math::wave_x1000(750, 1000) == -1000);
    assert(modulation_math::wave_x1000(500, 1000) == 0);
    assert(modulation_math::amplitude_duty(50, 0, 1000) == 25);
    assert(modulation_math::amplitude_duty(50, 250, 1000) == 50);
    assert(modulation_math::amplitude_duty(50, 750, 1000) == 0);
    assert(modulation_math::frequency_hz_x100(1000000UL, 500, 2000) == 1100000UL);
    assert(modulation_math::frequency_hz_x100(1000000UL, 1500, 2000) == 900000UL);
    assert(modulation_math::frequency_hz_x100(24UL, 500, 2000) > 0);
    return 0;
}
