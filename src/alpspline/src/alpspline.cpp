#include "alpspline.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

Point3D InterpolateBetweenPoints(SplinePoint* sp0, SplinePoint* sp1, f64 t) {
    Point3D p0 = sp0->position;
    Vector3D v0 = sp0->velocity;
    Point3D p1 = sp1->position;
    Vector3D v1 = sp1->velocity;

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

static Vector3D VelocityAtParam(CubicSpline* spline, f64 param) {
    f64 twoH = 2.0 * h;
    Point3D p0 = Interpolate(spline, param - h);
    Point3D p1 = Interpolate(spline, param + h);
    f64 dx = (p1.x - p0.x) / twoH;
    f64 dy = (p1.y - p0.y) / twoH;
    f64 dz = (p1.z - p0.z) / twoH;
    return (Vector3D){dx, dy, dz};
}

static f64 PointArcLength(CubicSpline* spline, f64 param) {
    Vector3D v = VelocityAtParam(spline, param);
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

ParamToArcLengthTable MapParamsToArcLength(CubicSpline* spline, f64 stepSize,
                                           MallocFn mallocFn) {
    ParamToArcLengthTable pToAL = (ParamToArcLengthTable){};

    f64 maxT = (f64)(spline->nPoints - 1);
    u32 nSteps = maxT / stepSize + 1;
    pToAL.nSteps = nSteps;
    pToAL.stepSize = stepSize;
    pToAL.arcLengths = (f64*)mallocFn(sizeof(f64) * pToAL.nSteps);

    f64 t = 0.0;
    f64 arcLength = 0.0;
    u32 index = 0;
    pToAL.arcLengths[index] = 0.0;
    ++index;

    while (index < (nSteps - 1)) {
        f64 leftValue = PointArcLength(spline, t);
        f64 midValue = PointArcLength(spline, t + stepSize * 0.5);
        f64 rightValue = PointArcLength(spline, t + stepSize);

        f64 midArea = midValue * stepSize;
        f64 trapArea = 0.5 * (leftValue + rightValue) * stepSize;

        arcLength += (2.0 * midArea + trapArea) / 3.0;

        pToAL.arcLengths[index] = arcLength;

        t += stepSize;
        ++index;
    }

    f64 leftValue = PointArcLength(spline, 1.0 - stepSize);
    f64 midValue = PointArcLength(spline, 1.0 - stepSize * 0.5);
    f64 rightValue = PointArcLength(spline, 1.0 - 5e-7);

    f64 midArea = midValue * stepSize;
    f64 trapArea = 0.5 * (leftValue + rightValue) * stepSize;

    arcLength += (2.0 * midArea + trapArea) / 3.0;

    pToAL.arcLengths[index] = arcLength;

    return pToAL;
}

void DestroyParamToArcLengthTable(ParamToArcLengthTable* table,
                                  FreeFn freeFn = free) {
    freeFn(table->arcLengths);
    *table = (ParamToArcLengthTable){};
}

f64 ParamToArcLength(ParamToArcLengthTable* palt, f64 param) {
    u32 index = (u32)(param / palt->stepSize);
    f64 remainder = param - (f64)index*palt->stepSize;
    f64 lowerBound = palt->arcLengths[index];
    f64 upperBound;
    if (index < palt->nSteps - 2) {
       upperBound = palt->arcLengths[index + 1]; 
    } else {
       return palt->arcLengths[index];
    }
    f64 r = remainder / (upperBound - lowerBound);

    return (1 - r)*lowerBound + r*upperBound;
}

bool ArcLengthToParam(ParamToArcLengthTable* palt, f64 arcLength,
                      f64* outParam) {
    u32 nSteps = palt->nSteps;
    f64* arcLengths = palt->arcLengths;
    f64 stepSize = palt->stepSize;

    u32 foundIndex = 0;
    for (u32 jump = nSteps / 2; jump >= 1; jump /= 2) {
        while (foundIndex + jump < nSteps &&
               arcLengths[foundIndex + jump] <= arcLength) {
            foundIndex += jump;
        }
    }
    if (foundIndex == nSteps) return false;

    f64 left = arcLengths[foundIndex];
    f64 right = arcLengths[foundIndex + 1];

    f64 r = (arcLength - left) / (right - left);

    *outParam =
        (1.0 - r) * foundIndex * stepSize + r * (foundIndex + 1) * stepSize;

    return true;
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
    *spline = {};
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

Point3D Interpolate(CubicSpline* spline, f64 param) {
    u32 paramFloor = (u32)param;
    SplinePoint sp0 = spline->points[paramFloor];
    SplinePoint sp1 = spline->points[paramFloor + 1];
    f64 t = param - paramFloor;

    return InterpolateBetweenPoints(&sp0, &sp1, t);
}

// --------- ALPSpline --------

ALPSpline CreateALPSpline(CubicSpline* sourceSpline, MallocFn mallocFn) {
    ALPSpline alpSpline = {};

    f64 stepSize = 0.001;
    ParamToArcLengthTable palt = MapParamsToArcLength(sourceSpline, stepSize);

    f64 sourceSplineLength = palt.arcLengths[palt.nSteps - 1];

    alpSpline.subSplineLength = sourceSplineLength / 100.0;
    alpSpline.nPoints = (u32)(sourceSplineLength / alpSpline.subSplineLength) + 1;
    alpSpline.points = (SplinePoint*)mallocFn(sizeof(SplinePoint) * alpSpline.nPoints);

    f64 arcLength = 0.0;
    f64 param;
    for(u32 i = 0; i < alpSpline.nPoints; ++i) {
        ArcLengthToParam(&palt, arcLength, &param);
        alpSpline.points[i].position = Interpolate(sourceSpline, param);

        Vector3D velocity = VelocityAtParam(sourceSpline, param);
        f64 length = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
        velocity = (Vector3D) {
            alpSpline.subSplineLength * velocity.x / length, alpSpline.subSplineLength * velocity.y / length, alpSpline.subSplineLength * velocity.z / length
        };
        alpSpline.points[i].velocity = velocity;

        arcLength += alpSpline.subSplineLength;
    }

    return alpSpline;
}

void DestroyALPSpline(ALPSpline* spline, FreeFn freeFn) {
    freeFn(spline->points);
    *spline = {};
}

Point3D InterpolateByArcLength(ALPSpline* spline, f64 arcLength) {
    // todo: arcLength out of range
    u32 firstPointIndex = (u32)(arcLength / spline->subSplineLength);
    f32 t = (arcLength - firstPointIndex * spline->subSplineLength) / spline->subSplineLength;
    return InterpolateBetweenPoints(&spline->points[firstPointIndex], &spline->points[firstPointIndex + 1], t);
}

Point3D InterpolateByParam(ALPSpline* spline, f64 param) {
    u32 paramFloor = (u32)param;
    SplinePoint sp0 = spline->points[paramFloor];
    SplinePoint sp1 = spline->points[paramFloor + 1];
    f64 t = param - paramFloor;

    return InterpolateBetweenPoints(&sp0, &sp1, t);
}
