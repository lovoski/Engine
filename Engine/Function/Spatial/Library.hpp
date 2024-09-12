#pragma once

int NoDivTriTriIsect(float V0[3], float V1[3], float V2[3], float U0[3],
                     float U1[3], float U2[3]);

int intersect_triangle3(double orig[3], double dir[3], double vert0[3],
                        double vert1[3], double vert2[3], double *t, double *u,
                        double *v);

int triBoxOverlap(float boxcenter[3], float boxhalfsize[3],
                  float triverts[3][3]);