#include "maps.h"
#include "perlinNoise.h"

#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HEIGHT 720
#define WIDTH 1280

#define CAMERA_SPEED 0.01
#define ZOOM_SPEED 1
#define ORIGIN_OFFSET .5

double size = 1;

int *permutation_table;

Vector3 target = {0};
Vector3 origin = {-ORIGIN_OFFSET, -ORIGIN_OFFSET, -ORIGIN_OFFSET};
Vector3 camera_start_pos = {20, 20, 20};
Vector3 camera_start_up = {0, 1, 0};

const int chuck_size = 10;
const int subdivision = 2;
const int world_height = 10;

typedef struct {
    int x;
    int z;
    Mesh plane;
    Model model;
    Matrix matrix;
    Material material;
} Chunk;

void print_vec(Vector3 vec) {
    printf("Cube 0, vertex 0: (%f, %f, %f)\n", vec.x, vec.y, vec.z);
}

Vector3 matrix_to_vec3(Matrix m) {
    return (Vector3){m.m12, m.m13, m.m14};
}

Chunk generate_chunk(int x, int z) {
    Chunk c;

    c.plane = (Mesh){0};
    c.plane.vertexCount = subdivision * 2 * 3;
    c.plane.triangleCount = subdivision * 2;
    c.plane.vertices = (float *)malloc(sizeof(float) * c.plane.vertexCount * 3);

    int vert_count = 0;
    for (size_t i = 0; i < subdivision; i++) {
        for (size_t j = 0; j < subdivision; j++) {
            float offset_x = chuck_size / subdivision * (i + 1);
            float offset_z = chuck_size / subdivision * (j + 1);

            printf("x: %f, z: %f\n", offset_x, offset_z);

            c.plane.vertices[vert_count + 0] = -0.5 * offset_x;
            c.plane.vertices[vert_count + 1] = 1.0f;
            c.plane.vertices[vert_count + 2] = -0.5f* offset_z;

            c.plane.vertices[vert_count + 3] = 0.5f  * offset_x;
            c.plane.vertices[vert_count + 4] = 1.0f;
            c.plane.vertices[vert_count + 5] = 0.5f * offset_z;

            c.plane.vertices[vert_count + 6] = 0.5f * offset_x;
            c.plane.vertices[vert_count + 7] = 1.0f;
            c.plane.vertices[vert_count + 8] = -0.5f * offset_z;

            c.plane.vertices[vert_count + 9] = -0.5f * offset_x;
            c.plane.vertices[vert_count + 10] = 1.0f;
            c.plane.vertices[vert_count + 11] = 0.5f * offset_z;

            c.plane.vertices[vert_count + 12] = 0.5f * offset_x;
            c.plane.vertices[vert_count + 13] = 1.0f;
            c.plane.vertices[vert_count + 14] = 0.5f * offset_z;

            c.plane.vertices[vert_count + 15] = -0.5f * offset_x;
            c.plane.vertices[vert_count + 16] = 1.0f;
            c.plane.vertices[vert_count + 17] = -0.5f * offset_z;

            vert_count += 18;
        }
    }

    c.material = LoadMaterialDefault();
    c.matrix = MatrixIdentity();

    c.matrix = MatrixTranslate(x * chuck_size, 0, z * chuck_size);

    c.material.maps[MATERIAL_MAP_DIFFUSE].color = GREEN;

    UploadMesh(&c.plane, false);
    c.model = LoadModelFromMesh(c.plane);
    return c;
}

void render_chunks(Chunk *chunks, int world_size) {
    for (size_t i = 0; i < world_size; i++) {
        DrawMesh(chunks[i].plane, chunks[i].material, chunks[i].matrix);
        DrawModelWires(chunks[i].model, matrix_to_vec3(chunks[i].matrix), 1, BLACK);
    }
}

void unload_world(Chunk *chunks, int world_size) {
    for (size_t i = 0; i < world_size; i++) {
        UnloadMaterial(chunks[i].material);
        UnloadMesh(chunks[i].plane);
        UnloadModel(chunks[i].model);
    }
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIDTH, HEIGHT, "WorldGen");
    SetTargetFPS(120);

    srand(1);
    // srand(time(NULL));

    permutation_table = make_permutation();

    int world_size = 1;
    Chunk *chunks = (Chunk *)malloc(sizeof(Chunk) * world_size);

    Mesh mesh = GenMeshPlane(chuck_size, chuck_size, 10, 10);
    Model model = LoadModelFromMesh(mesh);
    Material matt = LoadMaterialDefault();
    matt.maps[MATERIAL_MAP_DIFFUSE].color = RED;



    chunks[0] = generate_chunk(0, 0);
    // chunks[1] = generate_chunk(1, 0);
    // chunks[2] = generate_chunk(0, 1);
    // chunks[3] = generate_chunk(1, 1);


    Camera3D camera = {
        .position = camera_start_pos,
        .target = target,
        .up = camera_start_up,
        .fovy = 30,
        .projection = CAMERA_PERSPECTIVE};

    while (!WindowShouldClose()) {

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&camera, -dist * ZOOM_SPEED);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseDelta = GetMouseDelta();
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x > 70) {
                CameraYaw(&camera, -mouseDelta.x * CAMERA_SPEED, true);
                CameraPitch(&camera, -mouseDelta.y * CAMERA_SPEED, true, true, false);
            }
        }

        if (IsKeyPressed(KEY_R)) {
            camera.position = camera_start_pos;
            camera.up = camera_start_up;
        }

        // clang-format off
        BeginDrawing();
            ClearBackground(GetColor(0x181818AA));
            BeginMode3D(camera);
 
            render_chunks(chunks, world_size);

            DrawMesh(mesh, matt, MatrixTranslate(1 * chuck_size, 0, 0));
            DrawModelWires(model, matrix_to_vec3(MatrixTranslate(1 * chuck_size, 0, 0)), 1, BLACK);
            

            EndMode3D();
            DrawFPS(10, 10);

        EndDrawing();
        // clang-format on
    }

    unload_world(chunks, world_size);
    free(chunks);
    CloseWindow();

    return 0;
}