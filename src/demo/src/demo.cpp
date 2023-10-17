#include "alpspline.h"

#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "raylib.h"
#include "raymath.h"

int main(int argc, char** argv) {
    i32 screenWidth = 1920;
    i32 screenHeight = 1024;

    InitWindow(screenWidth, screenHeight, "libjam demo");
    SetTargetFPS(60);

    Camera2D camera = (Camera2D){(Vector2){}, (Vector2){}, 0.0f, 1.0f};

    CubicSpline spline = CreateCubicSpline(2);
    spline.points[0] = (SplinePoint){
        (Point3D){300, 500, 0},
        (Vector3D){100, 0},
    };
    spline.points[1] =
        (SplinePoint){(Point3D){1300, 500, 0}, (Vector3D){100, 0}};

    float* selectedVector = nullptr;
    while (!WindowShouldClose()) {
        {
            // Handle adding and deleting spline points.
            if (IsKeyPressed(KEY_A)) {
                Point3D lastPosition =
                    spline.points[spline.nPoints - 1].position;
                Vector3D lastVelocity =
                    spline.points[spline.nPoints - 1].velocity;
                ChangeNumberOfSplinePoints(&spline, spline.nPoints + 1);
                spline.points[spline.nPoints - 1] =
                    (SplinePoint){(Point3D){lastPosition.x + lastVelocity.x,
                                            lastPosition.y + lastVelocity.y, 0},
                                  lastVelocity};
            }

            if (IsKeyPressed(KEY_D) && spline.nPoints > 2) {
                ChangeNumberOfSplinePoints(&spline, spline.nPoints - 1);
            }

            if (IsKeyPressed(KEY_T)) {
                f64 stepSize = 0.001;
                ParamToArcLengthTable palt = MapParamsToArcLength(&spline, stepSize);
                for (i32 i = 0; i < palt.nSteps; ++i) {
                    TraceLog(LOG_INFO, "%d: %f -> %f", i, (i + 1)*palt.stepSize, palt.arcLengths[i]);
                }
            }

            {  // Handle changing points position etc.
                Vector2 mousePosition = GetMousePosition();

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    selectedVector = nullptr;
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    for (u32 i = 0; i < spline.nPoints; ++i) {
                        Point3D position = spline.points[i].position;
                        Vector3D velocity = spline.points[i].velocity;
                        i32 vGrabX = position.x + velocity.x;
                        i32 vGrabY = position.y + velocity.y;
                        if (abs(mousePosition.x - vGrabX) < 20 &&
                            abs(mousePosition.y - vGrabY) < 20) {
                            selectedVector = (float*)&spline.points[i].velocity;
                        } else if (abs(mousePosition.x - position.x) < 20 &&
                                   abs(mousePosition.y - position.y) < 20) {
                            selectedVector = (float*)&spline.points[i].position;
                        }
                    }
                }

                if (selectedVector) {
                    Vector2 mouseDelta = GetMouseDelta();
                    selectedVector[0] += mouseDelta.x;
                    selectedVector[1] += mouseDelta.y;
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);

        for (u32 i = 0; i < spline.nPoints; ++i) {
            Point3D position = spline.points[i].position;
            Vector3D velocity = spline.points[i].velocity;
            DrawCircle(position.x, position.y, 10, GRAY);
            DrawLineEx(
                (Vector2){(f32)position.x, (f32)position.y},
                (Vector2){(f32)(position.x + velocity.x), (f32)(position.y + velocity.y)},
                3.0f, PURPLE
                );
        }

        float t = 0.0f;
        u32 nDrawn = 100 * spline.nPoints;
        float step = ((spline.nPoints - 1) * 1.0f) / (float)nDrawn;
        for (i32 i = 0; i < nDrawn; ++i) {
            Point3D pointOnSpline = Evaluate(&spline, t);

            DrawCircle(pointOnSpline.x, pointOnSpline.y, 2, BLACK);

            t += step;
        }

        EndMode2D();
        EndDrawing();
    }

    DestroyCubicSpline(&spline);

    return 0;
}
