#include "../../ResoNet_Diagnostic_Nano/effector_runtime.h"
#include <assert.h>

using namespace ResoNetDiagnostic;

static EffectorEvent event(EventType type, EffectorId id,
                           uint16_t hz = 1000, uint8_t duty = 25,
                           uint16_t duration = 500) {
    EffectorEvent e = { type, id, hz, duty, duration };
    return e;
}

int main() {
    EffectorRuntime runtime;
    assert(runtime.queued() == 0);
    assert(runtime.enqueue(event(EVENT_CONFIGURE_PWM, EFFECTOR_IR_LED)));
    assert(runtime.processOne());
    const EffectorState* ir = runtime.state(EFFECTOR_IR_LED);
    assert(ir && ir->frequency_hz == 1000 && ir->duty_percent == 25);
    assert(!ir->requested_on);

    assert(runtime.enqueue(event(EVENT_START, EFFECTOR_PIEZO, 2200, 40, 250)));
    assert(runtime.processOne());
    const EffectorState* piezo = runtime.state(EFFECTOR_PIEZO);
    assert(piezo && piezo->requested_on && piezo->frequency_hz == 2200);

    assert(runtime.enqueue(event(EVENT_STOP, EFFECTOR_PIEZO)));
    assert(runtime.processOne());
    assert(!runtime.state(EFFECTOR_PIEZO)->requested_on);

    assert(!runtime.enqueue(event(EVENT_START, EFFECTOR_VIBRATOR, 0, 20, 100)));
    assert(!runtime.enqueue(event(EVENT_START, EFFECTOR_VIBRATOR, 1000, 101, 100)));
    assert(!runtime.enqueue(event(EVENT_START, EFFECTOR_VIBRATOR, 1000, 20, 0)));

    for (uint8_t i = 0; i < EFFECTOR_QUEUE_CAPACITY; ++i)
        assert(runtime.enqueue(event(EVENT_STOP, EFFECTOR_IR_LED)));
    assert(!runtime.enqueue(event(EVENT_STOP, EFFECTOR_IR_LED)));
    assert(runtime.overflowed());
    while (runtime.processOne()) {}
    assert(runtime.queued() == 0);

    runtime.resetOverflowFlag();
    assert(!runtime.overflowed());
    runtime.clearModel();
    assert(!runtime.state(EFFECTOR_IR_LED)->requested_on);
    assert(runtime.state(EFFECTOR_IR_LED)->frequency_hz == 0);
    return 0;
}
