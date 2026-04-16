#pragma once

#include "Models.h"

namespace GradeMath {
    // Compute course final given current grades and weights (wrapper around Course::ComputeFinal)
    float ComputeProjectedFinal(const Course &c);

    // Compute required final exam grade to reach `goal` (0-100). Returns required final (0-100+).
    // If finalWeight <= 0 returns a large value (FLT_MAX) to indicate impossible.
    float RequiredFinalForGoal(const Course &c, float goal);
}
