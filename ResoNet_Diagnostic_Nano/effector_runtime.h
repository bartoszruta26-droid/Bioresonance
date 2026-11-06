#ifndef RESONET_DIAGNOSTIC_EFFECTOR_RUNTIME_H
#define RESONET_DIAGNOSTIC_EFFECTOR_RUNTIME_H

#include <stdint.h>

namespace ResoNetDiagnostic {

static const uint8_t EFFECTOR_QUEUE_CAPACITY = 8;
static const uint8_t EFFECTOR_CHANNEL_COUNT = 3;

enum EffectorId : uint8_t {
    EFFECTOR_IR_LED = 0,
    EFFECTOR_PIEZO = 1,
    EFFECTOR_VIBRATOR = 2
};

enum EventType : uint8_t {
    EVENT_CONFIGURE_PWM = 1,
    EVENT_START = 2,
    EVENT_STOP = 3
};

struct EffectorEvent {
    EventType type;
    EffectorId effector;
    uint16_t frequency_hz;
    uint8_t duty_percent;
    uint16_t duration_ms;
};

struct EffectorState {
    uint16_t frequency_hz;
    uint8_t duty_percent;
    uint16_t duration_ms;
    bool requested_on;
};

struct PwmProfile {
    uint16_t frequency_hz;
    uint8_t duty_percent;
    uint16_t duration_ms;
};

/*
 * Diagnostic-only shadow runtime.
 *
 * Deliberately contains no pin numbers, Arduino GPIO/PWM calls, timer setup,
 * register access, or hardware driver. Events update RAM state only.
 * Physical outputs remain unavailable in this image.
 */
class EffectorRuntime {
public:
    EffectorRuntime() : head_(0), tail_(0), count_(0), overflowed_(false) {
        for (uint8_t i = 0; i < EFFECTOR_CHANNEL_COUNT; ++i) {
            states_[i].frequency_hz = 0;
            states_[i].duty_percent = 0;
            states_[i].duration_ms = 0;
            states_[i].requested_on = false;
        }
    }

    static bool validEffector(EffectorId id) {
        return static_cast<uint8_t>(id) < EFFECTOR_CHANNEL_COUNT;
    }

    static bool validProfile(const PwmProfile& profile) {
        return profile.frequency_hz > 0 &&
               profile.duty_percent <= 100 &&
               profile.duration_ms > 0;
    }

    bool enqueue(const EffectorEvent& event) {
        if (!validEffector(event.effector)) return false;
        if (event.type == EVENT_CONFIGURE_PWM || event.type == EVENT_START) {
            PwmProfile profile = {
                event.frequency_hz, event.duty_percent, event.duration_ms
            };
            if (!validProfile(profile)) return false;
        } else if (event.type != EVENT_STOP) {
            return false;
        }

        if (count_ >= EFFECTOR_QUEUE_CAPACITY) {
            overflowed_ = true;
            return false;
        }
        queue_[tail_] = event;
        tail_ = static_cast<uint8_t>((tail_ + 1U) % EFFECTOR_QUEUE_CAPACITY);
        ++count_;
        return true;
    }

    // Applies one queued event to the in-memory diagnostic model only.
    bool processOne() {
        if (count_ == 0) return false;
        const EffectorEvent event = queue_[head_];
        head_ = static_cast<uint8_t>((head_ + 1U) % EFFECTOR_QUEUE_CAPACITY);
        --count_;

        EffectorState& state = states_[static_cast<uint8_t>(event.effector)];
        if (event.type == EVENT_STOP) {
            state.requested_on = false;
        } else {
            state.frequency_hz = event.frequency_hz;
            state.duty_percent = event.duty_percent;
            state.duration_ms = event.duration_ms;
            state.requested_on = (event.type == EVENT_START);
        }
        return true;
    }

    const EffectorState* state(EffectorId id) const {
        return validEffector(id) ? &states_[static_cast<uint8_t>(id)] : 0;
    }

    uint8_t queued() const { return count_; }
    bool overflowed() const { return overflowed_; }

    void clearQueue() {
        head_ = tail_ = count_ = 0;
    }

    void resetOverflowFlag() { overflowed_ = false; }

    void clearModel() {
        clearQueue();
        for (uint8_t i = 0; i < EFFECTOR_CHANNEL_COUNT; ++i) {
            states_[i].frequency_hz = 0;
            states_[i].duty_percent = 0;
            states_[i].duration_ms = 0;
            states_[i].requested_on = false;
        }
    }

private:
    EffectorEvent queue_[EFFECTOR_QUEUE_CAPACITY];
    EffectorState states_[EFFECTOR_CHANNEL_COUNT];
    uint8_t head_;
    uint8_t tail_;
    uint8_t count_;
    bool overflowed_;
};

} // namespace ResoNetDiagnostic

#endif
