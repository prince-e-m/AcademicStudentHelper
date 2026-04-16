#include "GradeMath.h"
#include <cfloat>

namespace GradeMath {

float ComputeProjectedFinal(const Course &c) {
    return c.ComputeFinal();
}

float RequiredFinalForGoal(const Course &c, float goal) {
    float p = c.grade.prelim;
    float m = c.grade.midterm;
    float pwt = c.prelimWeight;
    float mwt = c.midtermWeight;
    float fwt = c.finalWeight;

    if (fwt <= 0.0f) return FLT_MAX; // no final component

    float current = p * pwt + m * mwt;
    float required = (goal - current) / fwt;
    return required;
}

} // namespace GradeMath
