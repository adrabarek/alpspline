#include <stdlib.h>
#include <stdint.h>

using MallocFn = void*(size_t);
using ReallocFn = void*(void*, size_t);
using FreeFn = void(void*);

using u32 = uint32_t;
using f32 = float;
using f64 = double;

struct Point3D {
    f64 x;
    f64 y;
    f64 z;
};
using Vector3D = Point3D;

struct SplinePoint {
    Point3D position;
    Vector3D velocity;
};

struct CubicSpline {
    SplinePoint* points;
    u32 nPoints;
};

CubicSpline CreateCubicSpline(u32 nPoints, MallocFn mallocFn = malloc);
void DestroyCubicSpline(CubicSpline* spline, FreeFn freeFn = free);
void ChangeNumberOfSplinePoints(CubicSpline* spline, u32 newNPoints,
                                ReallocFn reallocFn = realloc);
Point3D Interpolate(CubicSpline* spline, f64 param);

struct ALPSpline {
    SplinePoint* points;
    f64 subSplineLength;
    u32 nPoints;
};

ALPSpline CreateALPSpline(CubicSpline* sourceSpline, u32 nSubSplines = 100, MallocFn mallocFn = malloc);
void DestroyALPSpline(ALPSpline* spline, FreeFn freeFn = free);
Point3D InterpolateByParam(ALPSpline* spline, f64 param);
Point3D InterpolateByArcLength(ALPSpline* spline, f64 arcLength);

// PRIVATE, move to .cpp after testing.
struct ParamToArcLengthTable {
    f64 stepSize;
    f64* arcLengths;
    u32 nSteps;
};
ParamToArcLengthTable MapParamsToArcLength(CubicSpline* spline, f64 stepSize, MallocFn mallocFn = malloc);
f64 ParamToArcLength(ParamToArcLengthTable* palt, f64 param);
bool ArcLengthToParam(ParamToArcLengthTable* palt, f64 arcLength, f64* outParam);
