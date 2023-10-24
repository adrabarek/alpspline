#include "alpspline.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

constexpr f64 MAX_ERROR = 1e-3;

bool F64Eq(f64 a, f64 b, f64 maxError) {
    f64 diff = abs(a - b);
    bool result = diff < maxError;
    if (!result) {
        printf("F64Eq failed: a: %g; b: %g; diff: %g, max error: %g\n", a, b, diff, maxError);
    }
    return result;
}

f64 RandomCoord() {
    f64 randomF64 = (f64)rand()/(f64)RAND_MAX;
    f64 lower = -100000.0;
    f64 upper = 100000.0;
    return lower + randomF64*(upper - lower);
}

Point3D RandomPoint() {
    return (Point3D) { RandomCoord(), RandomCoord(), RandomCoord() };
}

Vector3D RandomVector() {
    return (Vector3D) { RandomCoord(), RandomCoord(), RandomCoord() };
}

void PrintPALT(ParamToArcLengthTable* palt) {
    printf("==== PALT ====\n");
    for(u32 i = 0; i < palt->nSteps; ++i) {
        printf("t = %g -> arc length %g\n", i * palt->stepSize, palt->arcLengths[i]);
    }
    printf("Step size: %g", palt->stepSize);
    printf("==== End of PALT ====\n");
}

void PrintPoint(Point3D point) {
    printf("[Point3D] x: %g, y: %g, z: %g\n", point.x, point.y, point.z);
}

void PrintVector(Vector3D vector) {
    printf("[Vector3D] x: %g, y: %g, z: %g\n", vector.x, vector.y, vector.z);
}

void PrintCubicSpline(CubicSpline* spline) {
    printf("==== Spline ====\n");
    for (u32 i = 0; i < spline->nPoints; ++i) {
        printf("%d:\n", i);
        printf("    ");
        PrintPoint(spline->points[i].position);
        printf("    ");
        PrintVector(spline->points[i].velocity);
    }
    printf("==== End of spline ====\n");
}

void PrintALPSpline(ALPSpline* spline) {
    printf("==== ALP spline ====\n");
    for (u32 i = 0; i < spline->nPoints; ++i) {
        printf("%d:\n", i);
        printf("    ");
        PrintPoint(spline->points[i].position);
        printf("    ");
        PrintVector(spline->points[i].velocity);
    }
    printf("==== End of spline ====\n");
}

CubicSpline RandomCubicSpline() {
    u32 npoints = 5;
    CubicSpline spline = CreateCubicSpline(npoints);
    for (u32 i = 0; i < npoints; ++i) {
        spline.points[i] = (SplinePoint) {
            RandomPoint(),
            RandomVector()
        };
    }

    return spline;
}

CubicSpline StraightCubicSpline() {
    CubicSpline spline = CreateCubicSpline(2);
    spline.points[0] = (SplinePoint){
        (Point3D){300, 500, 0},
        (Vector3D){100, 0},
    };
    spline.points[1] =
        (SplinePoint){(Point3D){1300, 500, 0}, (Vector3D){100, 0}};

    return spline;
}

void TestArcLengthIntegrationSimpleSpline() {
    CubicSpline spline = CreateCubicSpline(2);
    spline.points[0] = (SplinePoint){
        (Point3D){300, 500, 0},
        (Vector3D){100, 0},
    };
    spline.points[1] =
        (SplinePoint){(Point3D){1300, 500, 0}, (Vector3D){100, 0}};

    f64 stepSize = 0.001;
    ParamToArcLengthTable palt = MapParamsToArcLength(&spline, stepSize);
    f64 splineLength = palt.arcLengths[palt.nSteps - 1];
    assert(F64Eq(palt.arcLengths[0], 0.0, MAX_ERROR));
    assert(F64Eq(splineLength, 1000.0, MAX_ERROR));

    Point3D result;
    f64 reference;
    f64 param = 0.0;
    f64 arcLength = 0.0;
    f64 referenceX = spline.points[0].position.x;
    while(arcLength < splineLength) {
        ArcLengthToParam(&palt, arcLength, &param);
        result = Interpolate(&spline, param);
        f64 expectedX = result.x - referenceX;
        // printf("\nParam: %g, arc length: %g\n", param, arcLength);
        // printf("Error is: %g\n", expectedX - arcLength);
        assert(F64Eq(expectedX, arcLength, MAX_ERROR));
        arcLength += 2.32;
    }
}

void TestParamToArcLength() {
    { // Simple, straight spline.
        CubicSpline spline = StraightCubicSpline();
        f64 stepSize = 0.001;
        ParamToArcLengthTable palt = MapParamsToArcLength(&spline, stepSize);

        f64 param = 0.0;
        f64 arcLength = ParamToArcLength(&palt, param);
        assert(F64Eq(arcLength, 0.0, MAX_ERROR));

        param = 1.0;
        arcLength = ParamToArcLength(&palt, param);
        assert(F64Eq(arcLength, 1000.0, MAX_ERROR));

        param = 0.5;
        arcLength = ParamToArcLength(&palt, param);
        assert(F64Eq(arcLength, 500.0, MAX_ERROR));
    }

    { // Complex random spline.
        srand(12345);

        CubicSpline spline = RandomCubicSpline();
        f64 stepSize = 0.001;
        ParamToArcLengthTable palt = MapParamsToArcLength(&spline, stepSize);
//        PrintCubicSpline(&spline);
//        PrintPALT(&palt);

        f64 arcLength;
        f64 paramFromArcLength;
        f64 param = 0.0;
        while(param < (spline.nPoints - 1)) {
            arcLength = ParamToArcLength(&palt, param);
            ArcLengthToParam(&palt, arcLength, &paramFromArcLength);
            // printf("arcLength: %g, param: %g, computedParam: %g\n", arcLength, param, computedParam);
            assert(F64Eq(param, paramFromArcLength, MAX_ERROR));
            param += 0.01;
        }
    }
}

int main(int argc, char** argv) {
    TestArcLengthIntegrationSimpleSpline(); 
    TestParamToArcLength();
}
