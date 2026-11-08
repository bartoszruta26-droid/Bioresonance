#include "effector_resource_guard.h"
#include <cassert>

int main() {
    assert(effector_resources::diagnostic_allowed(false, false));
    assert(!effector_resources::diagnostic_allowed(true, false));
    assert(!effector_resources::diagnostic_allowed(false, true));
    assert(!effector_resources::diagnostic_allowed(true, true));
    return 0;
}
