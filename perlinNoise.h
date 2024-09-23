#ifndef MY_HEADER_H
#define MY_HEADER_H

#include <math.h>
#include <stdlib.h>

#define PERMUTATION_SIZE 256

typedef struct {
    double x;
    double y;
} Point;


void shuffle(int *arr, int arr_n) {
    for (int i = arr_n - 1; i > 0; i--) {
        int index = round(rand() % i);
        int temp = arr[i];

        arr[i] = arr[index];
        arr[index] = temp;
    }
}

int *make_permutation() {
    int *permutation = (int *)malloc(sizeof(int) * PERMUTATION_SIZE * 2);

    for (size_t i = 0; i < PERMUTATION_SIZE; i++) permutation[i] = i;

    shuffle(permutation, PERMUTATION_SIZE);

    for (size_t i = 0; i < PERMUTATION_SIZE; i++) permutation[PERMUTATION_SIZE + i] = permutation[i];

    return permutation;
}

Point get_constant_vector(int value_of_perm_table) {
    int h = value_of_perm_table & 3;

    if (h == 0)
        return (Point){1.0, 1.0};
    else if (h == 1)
        return (Point){-1.0, 1.0};
    else if (h == 2)
        return (Point){-1.0, -1.0};
    else
        return (Point){1.0, -1.0};
}

double fade_curve(double t) {
    return ((6 * t - 15) * t + 10) * t * t * t;
}

double lerp(double t, double a1, double a2) {
    return a1 + t * (a2 - a1);
}

double dot_product(Point p, Point p2) {
    return p.x * p2.x + p.y * p2.y;
}

double Noise2D(double x, double y, int *perm_table) {
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;

    double xf = x - floor(x);
    double yf = y - floor(y);

    Point topRight = (Point){.x = xf - 1.0, .y = yf - 1.0};
    Point topLeft = (Point){.x = xf, .y = yf - 1.0};
    Point bottomRight = (Point){.x = xf - 1.0, .y = yf};
    Point bottomLeft = (Point){.x = xf, .y = yf};

    int valueTopRight = perm_table[perm_table[X + 1] + Y + 1];
    int valueTopLeft = perm_table[perm_table[X] + Y + 1];
    int valueBottomRight = perm_table[perm_table[X + 1] + Y];
    int valueBottomLeft = perm_table[perm_table[X] + Y];

    double dotTopRight = dot_product(topRight, get_constant_vector(valueTopRight));
    double dotTopLeft = dot_product(topLeft, get_constant_vector(valueTopLeft));
    double dotBottomRight = dot_product(bottomRight, get_constant_vector(valueBottomRight));
    double dotBottomLeft = dot_product(bottomLeft, get_constant_vector(valueBottomLeft));

    double u = fade_curve(xf);
    double v = fade_curve(yf);

    return lerp(u,
                lerp(v, dotBottomLeft, dotTopLeft),
                lerp(v, dotBottomRight, dotTopRight));
}

double grad_3D(int hash, double x, double y, double z) {
    int h = hash & 15;        // CONVERT LO 4 BITS OF HASH CODE
    double u = h < 8 ? x : y; // INTO 12 GRADIENT DIRECTIONS.
    double v = h < 4 ? y : h == 12 || h == 14 ? x
                                              : z;

    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double Noise3D(double x, double y, double z, int *perm_table) {
    int X = (int)floor(x) & 255; // FIND UNIT CUBE THAT
    int Y = (int)floor(y) & 255; // CONTAINS POINT.
    int Z = (int)floor(z) & 255;

    x -= floor(x); // FIND RELATIVE X,Y,Z
    y -= floor(y); // OF POINT IN CUBE.
    z -= floor(z);

    double u = fade_curve(x); // COMPUTE FADE CURVES
    double v = fade_curve(y); // FOR EACH OF X,Y,Z.
    double w = fade_curve(z);

    int A = perm_table[X] + Y;
    int AA = perm_table[A] + Z;
    int AB = perm_table[A + 1] + Z; // HASH COORDINATES OF
    int B = perm_table[X + 1] + Y;
    int BA = perm_table[B] + Z;
    int BB = perm_table[B + 1] + Z; // THE 8 CUBE CORNERS,

    return lerp(w, lerp(v, lerp(u, grad_3D(perm_table[AA], x, y, z),        // AND ADD
                                grad_3D(perm_table[BA], x - 1, y, z)),      // BLENDED
                        lerp(u, grad_3D(perm_table[AB], x, y - 1, z),       // RESULTS
                             grad_3D(perm_table[BB], x - 1, y - 1, z))),    // FROM  8
                lerp(v, lerp(u, grad_3D(perm_table[AA + 1], x, y, z - 1),   // CORNERS
                             grad_3D(perm_table[BA + 1], x - 1, y, z - 1)), // OF CUBE
                     lerp(u, grad_3D(perm_table[AB + 1], x, y - 1, z - 1),
                          grad_3D(perm_table[BB + 1], x - 1, y - 1, z - 1))));
}

#endif // MY_HEADER_H