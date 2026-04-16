#include "GradeMath.h"

GradeReport calculateAcademicStatus(float p, float m, float target) {
    GradeReport report;
    report.targetGoal = target;

    float wP = 0.333f;
    float wM = 0.333f;
    float wF = 0.334f;

    report.neededForGoalMidterms = (target - (p * wP)) / (wM + wF);
    report.neededForGoalFinals = (target - (p * wP + m * wM)) / wF;
    report.projectedGrade = (p * wP) + (m * wM) + (((p + m) / 2.0f) * wF);
    report.neededToPass = (75.0f - (p * wP + m * wM)) / wF;
    report.safetyNetMidterm = (75.0f - (p * wP) - (60.0f * wF)) / wM;

    return report;
}
