#include <stdlib.h>
#include <stdint.h>

using MallocFn = void*(size_t);
using ReallocFn = void*(void*, size_t);
using FreeFn = void(void*);

using u32 = uint32_t;

struct Point3D {
    float x;
    float y;
    float z;
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
