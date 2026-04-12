#ifndef GRADEMATH_H
#define GRADEMATH_H

struct GradeReport {
    float projectedGrade;
    float neededForGoalMidterms;
    float neededForGoalFinals;
    float neededToPass;
    float safetyNetMidterm;
    float targetGoal;
};

GradeReport calculateAcademicStatus(float p, float m, float target);

#endif