#include "../../ResoNet_Nano/effector_presence.h"
#include <assert.h>
#include <stdint.h>

int main() {
    uint8_t mask = 0;
    mask = effector_presence::set(mask, 1, true);
    mask = effector_presence::set(mask, 3, true);
    assert(mask == static_cast<uint8_t>((1U << 1) | (1U << 3)));
    assert(effector_presence::primary(mask, 6) == 1);
    mask = effector_presence::set(mask, 1, false);
    assert(effector_presence::primary(mask, 6) == 3);
    mask = effector_presence::set(mask, 3, false);
    assert(effector_presence::primary(mask, 6) == 0);
    assert(effector_presence::set(mask, 0, true) == 0);
    assert(effector_presence::set(mask, 8, true) == 0);
    return 0;
}
