#ifndef EFFECTOR_RESOURCE_GUARD_H
#define EFFECTOR_RESOURCE_GUARD_H

namespace effector_resources {

// Diagnostics share physical timer/output resources with therapy generators.
// A diagnostic pulse is permitted only when both generators are stopped.
inline bool diagnostic_allowed(bool pwm_running, bool ir_running) {
    return !pwm_running && !ir_running;
}

}  // namespace effector_resources

#endif  // EFFECTOR_RESOURCE_GUARD_H
