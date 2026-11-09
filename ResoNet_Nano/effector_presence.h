#ifndef EFFector_PRESENCE_H
#define EFFector_PRESENCE_H

#include <stdint.h>

namespace effector_presence {

// Effectors use IDs 1..6; ID 0 is reserved for EFFECTOR_NONE.
static inline uint8_t set(uint8_t mask, uint8_t effector_id, bool present) {
    if (effector_id == 0U || effector_id > 7U) return mask;
    const uint8_t bit = static_cast<uint8_t>(1U << effector_id);
    return present ? static_cast<uint8_t>(mask | bit)
                   : static_cast<uint8_t>(mask & static_cast<uint8_t>(~bit));
}

static inline uint8_t primary(uint8_t mask, uint8_t max_effector_id) {
    for (uint8_t id = 1U; id <= max_effector_id && id < 8U; ++id) {
        if ((mask & static_cast<uint8_t>(1U << id)) != 0U) return id;
    }
    return 0U;
}

} // namespace effector_presence

#endif
