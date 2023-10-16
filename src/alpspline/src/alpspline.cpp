#include "alpspline.h"

#include <string.h>

CubicSpline CreateCubicSpline(u32 nPoints, MallocFn mallocFn) {
    SplinePoint* points = (SplinePoint*)mallocFn(sizeof(SplinePoint) * nPoints);

    CubicSpline result;
    result.points = points;
    result.nPoints = nPoints;

    return result;
}

void DestroyCubicSpline(CubicSpline* spline, FreeFn freeFn) {
    freeFn(spline->points);
    spline->points = nullptr;
    spline->nPoints = 0;
}

void ChangeNumberOfSplinePoints(CubicSpline* spline, u32 newNPoints,
                                ReallocFn reallocFn) {
    u32 oldNPoints = spline->nPoints;
    spline->nPoints = newNPoints;
    spline->points = (SplinePoint*)reallocFn(
        spline->points, sizeof(SplinePoint) * spline->nPoints);
    if (oldNPoints < newNPoints) {
        memset(spline->points + oldNPoints, 0, newNPoints - oldNPoints);
    }
}
