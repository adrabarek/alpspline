#include "alpspline.h"

#include <float.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

static f64 PointArcLength(CubicSpline* spline, f64 param) {
    f64 h = DBL_EPSILON;
    f64 twoH = 2.0 * h;
    Point3D p0 = Evaluate(spline, param - h);
    Point3D p1 = Evaluate(spline, param + h);
    f64 dx = (p1.x - p0.x) / twoH;
    f64 dy = (p1.y - p0.y) / twoH;
    f64 dz = (p1.z - p0.z) / twoH;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

ParamToArcLengthTable MapParamsToArcLength(CubicSpline* spline,
                                                  f64 stepSize,
                                                  MallocFn mallocFn) {
    ParamToArcLengthTable pToAL = (ParamToArcLengthTable){};

    f64 maxT = (f64)(spline->nPoints - 1);
    u32 nSteps = maxT / stepSize;
    pToAL.nSteps = nSteps;
    pToAL.stepSize = stepSize;
    pToAL.arcLengths = (f64*)mallocFn(sizeof(f64) * pToAL.nSteps);

    f64 t = 0.0f;
    f64 arcLength = 0.0f;
    u32 index = 0;
    while (t < maxT) {
        f64 leftValue = PointArcLength(spline, t);
        f64 midValue = PointArcLength(spline, t + stepSize * 0.5);
        f64 rightValue = PointArcLength(spline, t + stepSize);

        f64 midArea = midValue * stepSize;
        f64 trapArea = 0.5 * (leftValue + rightValue) * stepSize;

        arcLength += (2.0f * midArea + trapArea) / 3.0f;

        pToAL.arcLengths[index] = arcLength;

        t += stepSize;
        ++index;
    }

    return pToAL;
}

void DestroyParamToArcLengthTable(ParamToArcLengthTable* table, FreeFn freeFn = free) {
    freeFn(table->arcLengths);
    *table = (ParamToArcLengthTable) {};
}

// --------- CubicSpline --------

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

Point3D Evaluate(CubicSpline* spline, f64 param) {
    // printf("Evaluating param: %f\n", param);
    u32 paramFloor = (u32)param;
    SplinePoint sp0 = spline->points[paramFloor];
    SplinePoint sp1 = spline->points[paramFloor + 1];
    Point3D p0 = sp0.position;
    Vector3D v0 = sp0.velocity;
    Point3D p1 = sp1.position;
    Vector3D v1 = sp1.velocity;

    f64 t = param - paramFloor;
    f64 tSq = t * t;
    f64 oneMinusT = (1 - t);
    f64 oneMinusTSq = oneMinusT * oneMinusT;
    f64 twoT = 2 * t;

    f64 h00 = (1 + twoT) * oneMinusTSq;
    f64 h11 = t * oneMinusTSq;
    f64 h01 = tSq * (3 - twoT);
    f64 h10 = tSq * (t - 1);

    f64 x = h00 * p0.x + h10 * v1.x + h01 * p1.x + h11 * v0.x;
    f64 y = h00 * p0.y + h10 * v1.y + h01 * p1.y + h11 * v0.y;
    f64 z = h00 * p0.z + h10 * v1.z + h01 * p1.z + h11 * v0.z;

    return (Point3D){x, y, z};
}
