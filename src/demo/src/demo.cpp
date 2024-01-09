#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "alpspline.h"
#include "core.h"
#include "raylib.h"
#include "raymath.h"

ALPSpline* RecreateALP(ALPSpline* alp, CubicSpline* spline, u32 nALPpoints) {
    if (alp) {
        free(alp);
    }
    alp = (ALPSpline*)malloc(sizeof(ALPSpline));
    *alp = CreateALPSpline(spline, nALPpoints);
    return alp;
}

Vector3D NormalizeVector3D(Vector3D vector) {
    f64 invLength = 1.0f/sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    return (Vector3D) { vector.x * invLength, vector.y * invLength, vector.z * invLength };
}

Vector3D ScaleVector3D(Vector3D vector, f64 scale) {
    return (Vector3D) { vector.x * scale, vector.y * scale, vector.z * scale };
}

int main(int argc, char** argv) {
    i32 screenWidth = 1920;
    i32 screenHeight = 1024;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "libjam demo");
    SetTargetFPS(60);

    Color color0 = GetColor(0x3d30a2ff);
    Color color1 = GetColor(0xb15effff);
    Color color2 = GetColor(0xffa33cff);
    Color color3 = GetColor(0xfffb73ff);

    Camera2D camera = (Camera2D){(Vector2){}, (Vector2){}, 0.0f, 1.0f};

    CubicSpline spline = CreateCubicSpline(2);
    spline.points[0] = (SplinePoint){
        (Point3D){300, 500, 0},
        (Vector3D){300, 0},
    };
    spline.points[1] =
        (SplinePoint){(Point3D){1300, 500, 0}, (Vector3D){300, 0}};

    f64* selectedVector = nullptr;
    f64 selectedVectorScale = 1.0;
    f64 velocityScale = 3.0;
    ALPSpline* alp = nullptr;
    u32 nALPpoints = 10;
    f64 arcLengthPhase = 0.0;
    bool arcLengthMode = false;

    while (!WindowShouldClose()) {
        {
            // Handle input
            if (arcLengthMode) {
                if (IsKeyPressed(KEY_V)) {
                    arcLengthMode = false;
                }

                if (IsKeyPressed(KEY_A)) {
                    nALPpoints += 1;
                    alp = RecreateALP(alp, &spline, nALPpoints);
                }

                if (IsKeyPressed(KEY_D)) {
                    nALPpoints -= 1;
                    alp = RecreateALP(alp, &spline, nALPpoints);
                }
            } else {
                if (IsKeyPressed(KEY_A)) {
                    Point3D lastPosition =
                        spline.points[spline.nPoints - 1].position;
                    Vector3D lastVelocity =
                        ScaleVector3D(NormalizeVector3D(spline.points[spline.nPoints - 1].velocity), 100.0);
                    ChangeNumberOfSplinePoints(&spline, spline.nPoints + 1);
                    spline.points[spline.nPoints - 1] =
                        (SplinePoint){(Point3D){lastPosition.x + lastVelocity.x,
                                                lastPosition.y + lastVelocity.y, 0},
                                      lastVelocity};
                }

                if (IsKeyPressed(KEY_D) && spline.nPoints > 2) {
                    ChangeNumberOfSplinePoints(&spline, spline.nPoints - 1);
                }

                if (IsKeyPressed(KEY_V)) {
                    alp = RecreateALP(alp, &spline, nALPpoints);
                    arcLengthMode = !arcLengthMode;
                }

                {  // Dragging source spline points and velocity controls.
                    Vector2 mousePosition = GetMousePosition();

                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                        selectedVector = nullptr;
                    }

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        for (u32 i = 0; i < spline.nPoints; ++i) {
                            Point3D position = spline.points[i].position;
                            Vector3D velocity = spline.points[i].velocity;
                            i32 vGrabX = position.x + velocity.x/velocityScale;
                            i32 vGrabY = position.y + velocity.y/velocityScale;
                            if (abs(mousePosition.x - vGrabX) < 20 &&
                                abs(mousePosition.y - vGrabY) < 20) {
                                selectedVector = (f64*)&spline.points[i].velocity;
                                selectedVectorScale = velocityScale;
                            } else if (abs(mousePosition.x - position.x) < 20 &&
                                       abs(mousePosition.y - position.y) < 20) {
                                selectedVector = (f64*)&spline.points[i].position;
                                selectedVectorScale = 1.0;
                            }
                        }
                    }

                    if (selectedVector) {
                        Vector2 mouseDelta = GetMouseDelta();
                        selectedVector[0] += mouseDelta.x * selectedVectorScale;
                        selectedVector[1] += mouseDelta.y * selectedVectorScale;
                    }
                }
            }

        }

        BeginDrawing();
        ClearBackground(color0);
        BeginMode2D(camera);
        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

        // Draw the source spline.
        float t = 0.0f;
        u32 nDrawn = 100 * spline.nPoints;
        float step = (spline.nPoints - 1.0f) / (f32)nDrawn;
        Point3D prevPoint = Interpolate(&spline, 0.0);
        Point3D currPoint;
        Color splineColor = arcLengthMode ? GRAY : color3;
        for (i32 i = 1; i < nDrawn; ++i) {
            currPoint = Interpolate(&spline, t);

            DrawLineV((Vector2){(f32)prevPoint.x, (f32)prevPoint.y},
                      (Vector2){(f32)currPoint.x, (f32)currPoint.y},
                      splineColor);

            prevPoint = currPoint;

            t += step;
        }

        if (arcLengthMode) {
            // Draw the ALP spline.
            u32 nDrawn = 100 * alp->nPoints;
            float step = (alp->nPoints - 1.0f) / (f32)nDrawn;
            Point3D prevPoint = InterpolateByParam(alp, 0.0);
            Point3D currPoint;
            for (float t = 0.0f; t < (alp->nPoints - 1.0f); t += step) {
                currPoint = InterpolateByParam(alp, t);

                DrawLineV((Vector2){(f32)prevPoint.x, (f32)prevPoint.y},
                          (Vector2){(f32)currPoint.x, (f32)currPoint.y},
                          color3);

                prevPoint = currPoint;

                t += step;
            }

            // Draw the ALP spline points.
            for (u32 i = 0; i < alp->nPoints; ++i) {
                Point3D p = alp->points[i].position;
                DrawCircleV((Vector2){(f32)p.x, (f32)p.y}, 5.0f, color1);
            }

            // Draw points moving along the spline at constant speed.
            f64 alpLength = (alp->nPoints - 1) * alp->subSplineLength;
            f64 arcLengthPointsGap = 50.0; 
            for (f64 i = 0.0; i + arcLengthPhase < alpLength; i += arcLengthPointsGap) {
                Point3D arcLengthPoint =
                    InterpolateByArcLength(alp, i + arcLengthPhase);
                DrawCircleV((Vector2){(f32)arcLengthPoint.x, (f32)arcLengthPoint.y},
                            5.0, RED);
            }
            arcLengthPhase += 1.0;
            if (arcLengthPhase > arcLengthPointsGap) arcLengthPhase = 0.0;
        } else {
            // Draw the velocity handles.
            for (u32 i = 0; i < spline.nPoints; ++i) {
                Point3D position = spline.points[i].position;
                DrawCircle(position.x, position.y, 10, color1);

                Vector3D velocity = spline.points[i].velocity;
                Vector2 lineStart = (Vector2){(f32)position.x, (f32)position.y};
                Vector2 lineEnd = (Vector2){(f32)(position.x + velocity.x/velocityScale),
                                            (f32)(position.y + velocity.y/velocityScale)};
                DrawLineEx(lineStart, lineEnd, 1.0f, color1);
                DrawCircleV(lineEnd, 4.0f, color1);
                DrawCircleV(lineEnd, 3.0f, color0);
            }
        }

        if (arcLengthMode) {
            DrawText("[A] Add subspline (improve accuracy)", 5, 5, 18,
                     BLACK);
            DrawText("[D] Remove subspline (reduce accuracy)", 5, 28, 18,
                     BLACK);
            DrawText("[V] Edit source spline", 5, 51, 18,
                     BLACK);
        } else {
            DrawText("[A] Add point", 5, 5, 18,
                     BLACK);
            DrawText("[D] Delete point", 5, 28, 18,
                     BLACK);
            DrawText("[V] Parametrize by arc length", 5, 51, 18,
                     BLACK);
        }

        EndBlendMode();
        EndMode2D();
        EndDrawing();
    }

    DestroyCubicSpline(&spline);

    return 0;
}
