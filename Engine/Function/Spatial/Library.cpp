/* Ray-Triangle Intersection Test Routines          */
/* Different optimizations of my and Ben Trumbore's */
/* code from journals of graphics tools (JGT)       */
/* http://www.acm.org/jgt/                          */
/* by Tomas Moller, May 2000                        */

/*
Copyright 2020 Tomas Akenine-MÃ¶ller

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <math.h>

#define EPSILON 0.000001
#define CROSS(dest, v1, v2)                                                    \
  dest[0] = v1[1] * v2[2] - v1[2] * v2[1];                                     \
  dest[1] = v1[2] * v2[0] - v1[0] * v2[2];                                     \
  dest[2] = v1[0] * v2[1] - v1[1] * v2[0];
#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])
#define SUB(dest, v1, v2)                                                      \
  dest[0] = v1[0] - v2[0];                                                     \
  dest[1] = v1[1] - v2[1];                                                     \
  dest[2] = v1[2] - v2[2];

/* the original jgt code */
int intersect_triangle(double orig[3], double dir[3], double vert0[3],
                       double vert1[3], double vert2[3], double *t, double *u,
                       double *v) {
  double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
  double det, inv_det;

  /* find vectors for two edges sharing vert0 */
  SUB(edge1, vert1, vert0);
  SUB(edge2, vert2, vert0);

  /* begin calculating determinant - also used to calculate U parameter */
  CROSS(pvec, dir, edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  det = DOT(edge1, pvec);

  if (det > -EPSILON && det < EPSILON)
    return 0;
  inv_det = 1.0 / det;

  /* calculate distance from vert0 to ray origin */
  SUB(tvec, orig, vert0);

  /* calculate U parameter and test bounds */
  *u = DOT(tvec, pvec) * inv_det;
  if (*u < 0.0 || *u > 1.0)
    return 0;

  /* prepare to test V parameter */
  CROSS(qvec, tvec, edge1);

  /* calculate V parameter and test bounds */
  *v = DOT(dir, qvec) * inv_det;
  if (*v < 0.0 || *u + *v > 1.0)
    return 0;

  /* calculate t, ray intersects triangle */
  *t = DOT(edge2, qvec) * inv_det;

  return 1;
}

/* code rewritten to do tests on the sign of the determinant */
/* the division is at the end in the code                    */
int intersect_triangle1(double orig[3], double dir[3], double vert0[3],
                        double vert1[3], double vert2[3], double *t, double *u,
                        double *v) {
  double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
  double det, inv_det;

  /* find vectors for two edges sharing vert0 */
  SUB(edge1, vert1, vert0);
  SUB(edge2, vert2, vert0);

  /* begin calculating determinant - also used to calculate U parameter */
  CROSS(pvec, dir, edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  det = DOT(edge1, pvec);

  if (det > EPSILON) {
    /* calculate distance from vert0 to ray origin */
    SUB(tvec, orig, vert0);

    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec);
    if (*u < 0.0 || *u > det)
      return 0;

    /* prepare to test V parameter */
    CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec);
    if (*v < 0.0 || *u + *v > det)
      return 0;

  } else if (det < -EPSILON) {
    /* calculate distance from vert0 to ray origin */
    SUB(tvec, orig, vert0);

    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec);
    /*      printf("*u=%f\n",(float)*u); */
    /*      printf("det=%f\n",det); */
    if (*u > 0.0 || *u < det)
      return 0;

    /* prepare to test V parameter */
    CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec);
    if (*v > 0.0 || *u + *v < det)
      return 0;
  } else
    return 0; /* ray is parallell to the plane of the triangle */

  inv_det = 1.0 / det;

  /* calculate t, ray intersects triangle */
  *t = DOT(edge2, qvec) * inv_det;
  (*u) *= inv_det;
  (*v) *= inv_det;

  return 1;
}

/* code rewritten to do tests on the sign of the determinant */
/* the division is before the test of the sign of the det    */
int intersect_triangle2(double orig[3], double dir[3], double vert0[3],
                        double vert1[3], double vert2[3], double *t, double *u,
                        double *v) {
  double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
  double det, inv_det;

  /* find vectors for two edges sharing vert0 */
  SUB(edge1, vert1, vert0);
  SUB(edge2, vert2, vert0);

  /* begin calculating determinant - also used to calculate U parameter */
  CROSS(pvec, dir, edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  det = DOT(edge1, pvec);

  /* calculate distance from vert0 to ray origin */
  SUB(tvec, orig, vert0);
  inv_det = 1.0 / det;

  if (det > EPSILON) {
    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec);
    if (*u < 0.0 || *u > det)
      return 0;

    /* prepare to test V parameter */
    CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec);
    if (*v < 0.0 || *u + *v > det)
      return 0;

  } else if (det < -EPSILON) {
    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec);
    if (*u > 0.0 || *u < det)
      return 0;

    /* prepare to test V parameter */
    CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec);
    if (*v > 0.0 || *u + *v < det)
      return 0;
  } else
    return 0; /* ray is parallell to the plane of the triangle */

  /* calculate t, ray intersects triangle */
  *t = DOT(edge2, qvec) * inv_det;
  (*u) *= inv_det;
  (*v) *= inv_det;

  return 1;
}

/* code rewritten to do tests on the sign of the determinant */
/* the division is before the test of the sign of the det    */
/* and one CROSS has been moved out from the if-else if-else */
int intersect_triangle3(double orig[3], double dir[3], double vert0[3],
                        double vert1[3], double vert2[3], double *t, double *u,
                        double *v) {
  double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
  double det, inv_det;

  /* find vectors for two edges sharing vert0 */
  SUB(edge1, vert1, vert0);
  SUB(edge2, vert2, vert0);

  /* begin calculating determinant - also used to calculate U parameter */
  CROSS(pvec, dir, edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  det = DOT(edge1, pvec);

  /* calculate distance from vert0 to ray origin */
  SUB(tvec, orig, vert0);
  inv_det = 1.0 / det;

  CROSS(qvec, tvec, edge1);

  if (det > EPSILON) {
    *u = DOT(tvec, pvec);
    if (*u < 0.0 || *u > det)
      return 0;

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec);
    if (*v < 0.0 || *u + *v > det)
      return 0;

  } else if (det < -EPSILON) {
    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec);
    if (*u > 0.0 || *u < det)
      return 0;

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec);
    if (*v > 0.0 || *u + *v < det)
      return 0;
  } else
    return 0; /* ray is parallell to the plane of the triangle */

  *t = DOT(edge2, qvec) * inv_det;
  (*u) *= inv_det;
  (*v) *= inv_det;

  return 1;
}
/********************************************************/

/* AABB-triangle overlap test code                      */

/* by Tomas Akenine-Möller                              */

/* Function: int triBoxOverlap(float boxcenter[3],      */

/*          float boxhalfsize[3],float triverts[3][3]); */

/* History:                                             */

/*   2001-03-05: released the code in its first version */

/*   2001-06-18: changed the order of the tests, faster */

/*                                                      */

/* Acknowledgement: Many thanks to Pierre Terdiman for  */

/* suggestions and discussions on how to optimize code. */

/* Thanks to David Hunt for finding a ">="-bug!         */

/********************************************************/

/*
Copyright 2020 Tomas Akenine-Möller

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <math.h>

#include <stdio.h>

#define X 0

#define Y 1

#define Z 2

#define CROSS(dest, v1, v2)                                                    \
  dest[0] = v1[1] * v2[2] - v1[2] * v2[1];                                     \
  dest[1] = v1[2] * v2[0] - v1[0] * v2[2];                                     \
  dest[2] = v1[0] * v2[1] - v1[1] * v2[0];

#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])

#define SUB(dest, v1, v2)                                                      \
  dest[0] = v1[0] - v2[0];                                                     \
  dest[1] = v1[1] - v2[1];                                                     \
  dest[2] = v1[2] - v2[2];

#define FINDMINMAX(x0, x1, x2, min, max)                                       \
  min = max = x0;                                                              \
  if (x1 < min)                                                                \
    min = x1;                                                                  \
  if (x1 > max)                                                                \
    max = x1;                                                                  \
  if (x2 < min)                                                                \
    min = x2;                                                                  \
  if (x2 > max)                                                                \
    max = x2;

int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3]) // -NJMP-

{

  int q;

  float vmin[3], vmax[3], v;

  for (q = X; q <= Z; q++)

  {

    v = vert[q]; // -NJMP-

    if (normal[q] > 0.0f)

    {

      vmin[q] = -maxbox[q] - v; // -NJMP-

      vmax[q] = maxbox[q] - v; // -NJMP-

    }

    else

    {

      vmin[q] = maxbox[q] - v; // -NJMP-

      vmax[q] = -maxbox[q] - v; // -NJMP-
    }
  }

  if (DOT(normal, vmin) > 0.0f)
    return 0; // -NJMP-

  if (DOT(normal, vmax) >= 0.0f)
    return 1; // -NJMP-

  return 0;
}

/*======================== X-tests ========================*/

#define AXISTEST_X01(a, b, fa, fb)                                             \
  p0 = a * v0[Y] - b * v0[Z];                                                  \
  p2 = a * v2[Y] - b * v2[Z];                                                  \
  if (p0 < p2) {                                                               \
    min = p0;                                                                  \
    max = p2;                                                                  \
  } else {                                                                     \
    min = p2;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

#define AXISTEST_X2(a, b, fa, fb)                                              \
  p0 = a * v0[Y] - b * v0[Z];                                                  \
  p1 = a * v1[Y] - b * v1[Z];                                                  \
  if (p0 < p1) {                                                               \
    min = p0;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

/*======================== Y-tests ========================*/

#define AXISTEST_Y02(a, b, fa, fb)                                             \
  p0 = -a * v0[X] + b * v0[Z];                                                 \
  p2 = -a * v2[X] + b * v2[Z];                                                 \
  if (p0 < p2) {                                                               \
    min = p0;                                                                  \
    max = p2;                                                                  \
  } else {                                                                     \
    min = p2;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

#define AXISTEST_Y1(a, b, fa, fb)                                              \
  p0 = -a * v0[X] + b * v0[Z];                                                 \
  p1 = -a * v1[X] + b * v1[Z];                                                 \
  if (p0 < p1) {                                                               \
    min = p0;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)                                             \
  p1 = a * v1[X] - b * v1[Y];                                                  \
  p2 = a * v2[X] - b * v2[Y];                                                  \
  if (p2 < p1) {                                                               \
    min = p2;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p2;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

#define AXISTEST_Z0(a, b, fa, fb)                                              \
  p0 = a * v0[X] - b * v0[Y];                                                  \
  p1 = a * v1[X] - b * v1[Y];                                                  \
  if (p0 < p1) {                                                               \
    min = p0;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

int triBoxOverlap(float boxcenter[3], float boxhalfsize[3],
                  float triverts[3][3])

{

  /*    use separating axis theorem to test overlap between triangle and box */

  /*    need to test for overlap in these directions: */

  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the
   * triangle */

  /*       we do not even need to test these) */

  /*    2) normal of the triangle */

  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */

  /*       this gives 3x3=9 more tests */

  float v0[3], v1[3], v2[3];

  //   float axis[3];

  float min, max, p0, p1, p2, rad, fex, fey,
      fez; // -NJMP- "d" local variable removed

  float normal[3], e0[3], e1[3], e2[3];

  /* This is the fastest branch on Sun */

  /* move everything so that the boxcenter is in (0,0,0) */

  SUB(v0, triverts[0], boxcenter);

  SUB(v1, triverts[1], boxcenter);

  SUB(v2, triverts[2], boxcenter);

  /* compute triangle edges */

  SUB(e0, v1, v0); /* tri edge 0 */

  SUB(e1, v2, v1); /* tri edge 1 */

  SUB(e2, v0, v2); /* tri edge 2 */

  /* Bullet 3:  */

  /*  test the 9 tests first (this was faster) */

  fex = fabsf(e0[X]);

  fey = fabsf(e0[Y]);

  fez = fabsf(e0[Z]);

  AXISTEST_X01(e0[Z], e0[Y], fez, fey);

  AXISTEST_Y02(e0[Z], e0[X], fez, fex);

  AXISTEST_Z12(e0[Y], e0[X], fey, fex);

  fex = fabsf(e1[X]);

  fey = fabsf(e1[Y]);

  fez = fabsf(e1[Z]);

  AXISTEST_X01(e1[Z], e1[Y], fez, fey);

  AXISTEST_Y02(e1[Z], e1[X], fez, fex);

  AXISTEST_Z0(e1[Y], e1[X], fey, fex);

  fex = fabsf(e2[X]);

  fey = fabsf(e2[Y]);

  fez = fabsf(e2[Z]);

  AXISTEST_X2(e2[Z], e2[Y], fez, fey);

  AXISTEST_Y1(e2[Z], e2[X], fez, fex);

  AXISTEST_Z12(e2[Y], e2[X], fey, fex);

  /* Bullet 1: */

  /*  first test overlap in the {x,y,z}-directions */

  /*  find min, max of the triangle each direction, and test for overlap in */

  /*  that direction -- this is equivalent to testing a minimal AABB around */

  /*  the triangle against the AABB */

  /* test in X-direction */

  FINDMINMAX(v0[X], v1[X], v2[X], min, max);

  if (min > boxhalfsize[X] || max < -boxhalfsize[X])
    return 0;

  /* test in Y-direction */

  FINDMINMAX(v0[Y], v1[Y], v2[Y], min, max);

  if (min > boxhalfsize[Y] || max < -boxhalfsize[Y])
    return 0;

  /* test in Z-direction */

  FINDMINMAX(v0[Z], v1[Z], v2[Z], min, max);

  if (min > boxhalfsize[Z] || max < -boxhalfsize[Z])
    return 0;

  /* Bullet 2: */

  /*  test if the box intersects the plane of the triangle */

  /*  compute plane equation of triangle: normal*x+d=0 */

  CROSS(normal, e0, e1);

  // -NJMP- (line removed here)

  if (!planeBoxOverlap(normal, v0, boxhalfsize))
    return 0; // -NJMP-

  return 1; /* box and triangle overlaps */
}

/* Triangle/triangle intersection test routine,
 * by Tomas Moller, 1997.
 * See article "A Fast Triangle-Triangle Intersection Test",
 * Journal of Graphics Tools, 2(2), 1997
 * updated: 2001-06-20 (added line of intersection)
 *
 * int tri_tri_intersect(float V0[3],float V1[3],float V2[3],
 *                       float U0[3],float U1[3],float U2[3])
 *
 * parameters: vertices of triangle 1: V0,V1,V2
 *             vertices of triangle 2: U0,U1,U2
 * result    : returns 1 if the triangles intersect, otherwise 0
 *
 * Here is a version withouts divisions (a little faster)
 * int NoDivTriTriIsect(float V0[3],float V1[3],float V2[3],
 *                      float U0[3],float U1[3],float U2[3]);
 *
 * This version computes the line of intersection as well (if they are not
 * coplanar):
 * int tri_tri_intersect_with_isectline(float V0[3],float V1[3],float
 *V2[3], float U0[3],float U1[3],float U2[3],int *coplanar, float
 *isectpt1[3],float isectpt2[3]); coplanar returns whether the tris are coplanar
 * isectpt1, isectpt2 are the endpoints of the line of intersection
 */

/*
Copyright 2020 Tomas Akenine-Möller

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <math.h>

#define FABS(x) ((float)fabs(x)) /* implement as is fastest on your machine */

/* if USE_EPSILON_TEST is true then we do a check:
         if |dv|<EPSILON then dv=0.0;
   else no check is done (which is less robust)
*/
#define USE_EPSILON_TEST TRUE
#define EPSILON 0.000001

/* some macros */
#define CROSS(dest, v1, v2)                                                    \
  dest[0] = v1[1] * v2[2] - v1[2] * v2[1];                                     \
  dest[1] = v1[2] * v2[0] - v1[0] * v2[2];                                     \
  dest[2] = v1[0] * v2[1] - v1[1] * v2[0];

#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])

#define SUB(dest, v1, v2)                                                      \
  dest[0] = v1[0] - v2[0];                                                     \
  dest[1] = v1[1] - v2[1];                                                     \
  dest[2] = v1[2] - v2[2];

#define ADD(dest, v1, v2)                                                      \
  dest[0] = v1[0] + v2[0];                                                     \
  dest[1] = v1[1] + v2[1];                                                     \
  dest[2] = v1[2] + v2[2];

#define MULT(dest, v, factor)                                                  \
  dest[0] = factor * v[0];                                                     \
  dest[1] = factor * v[1];                                                     \
  dest[2] = factor * v[2];

#define SET(dest, src)                                                         \
  dest[0] = src[0];                                                            \
  dest[1] = src[1];                                                            \
  dest[2] = src[2];

/* sort so that a<=b */
#define SORT(a, b)                                                             \
  if (a > b) {                                                                 \
    float c;                                                                   \
    c = a;                                                                     \
    a = b;                                                                     \
    b = c;                                                                     \
  }

#define ISECT(VV0, VV1, VV2, D0, D1, D2, isect0, isect1)                       \
  isect0 = VV0 + (VV1 - VV0) * D0 / (D0 - D1);                                 \
  isect1 = VV0 + (VV2 - VV0) * D0 / (D0 - D2);

#define COMPUTE_INTERVALS(VV0, VV1, VV2, D0, D1, D2, D0D1, D0D2, isect0,       \
                          isect1)                                              \
  if (D0D1 > 0.0f) {                                                           \
    /* here we know that D0D2<=0.0 */                                          \
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
    ISECT(VV2, VV0, VV1, D2, D0, D1, isect0, isect1);                          \
  } else if (D0D2 > 0.0f) {                                                    \
    /* here we know that d0d1<=0.0 */                                          \
    ISECT(VV1, VV0, VV2, D1, D0, D2, isect0, isect1);                          \
  } else if (D1 * D2 > 0.0f || D0 != 0.0f) {                                   \
    /* here we know that d0d1<=0.0 or that D0!=0.0 */                          \
    ISECT(VV0, VV1, VV2, D0, D1, D2, isect0, isect1);                          \
  } else if (D1 != 0.0f) {                                                     \
    ISECT(VV1, VV0, VV2, D1, D0, D2, isect0, isect1);                          \
  } else if (D2 != 0.0f) {                                                     \
    ISECT(VV2, VV0, VV1, D2, D0, D1, isect0, isect1);                          \
  } else {                                                                     \
    /* triangles are coplanar */                                               \
    return coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2);                       \
  }

/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0, U0, U1)                                             \
  Bx = U0[i0] - U1[i0];                                                        \
  By = U0[i1] - U1[i1];                                                        \
  Cx = V0[i0] - U0[i0];                                                        \
  Cy = V0[i1] - U0[i1];                                                        \
  f = Ay * Bx - Ax * By;                                                       \
  d = By * Cx - Bx * Cy;                                                       \
  if ((f > 0 && d >= 0 && d <= f) || (f < 0 && d <= 0 && d >= f)) {            \
    e = Ax * Cy - Ay * Cx;                                                     \
    if (f > 0) {                                                               \
      if (e >= 0 && e <= f)                                                    \
        return 1;                                                              \
    } else {                                                                   \
      if (e <= 0 && e >= f)                                                    \
        return 1;                                                              \
    }                                                                          \
  }

#define EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2)                             \
  {                                                                            \
    float Ax, Ay, Bx, By, Cx, Cy, e, d, f;                                     \
    Ax = V1[i0] - V0[i0];                                                      \
    Ay = V1[i1] - V0[i1];                                                      \
    /* test edge U0,U1 against V0,V1 */                                        \
    EDGE_EDGE_TEST(V0, U0, U1);                                                \
    /* test edge U1,U2 against V0,V1 */                                        \
    EDGE_EDGE_TEST(V0, U1, U2);                                                \
    /* test edge U2,U1 against V0,V1 */                                        \
    EDGE_EDGE_TEST(V0, U2, U0);                                                \
  }

#define POINT_IN_TRI(V0, U0, U1, U2)                                           \
  {                                                                            \
    float a, b, c, d0, d1, d2;                                                 \
    /* is T1 completly inside T2? */                                           \
    /* check if V0 is inside tri(U0,U1,U2) */                                  \
    a = U1[i1] - U0[i1];                                                       \
    b = -(U1[i0] - U0[i0]);                                                    \
    c = -a * U0[i0] - b * U0[i1];                                              \
    d0 = a * V0[i0] + b * V0[i1] + c;                                          \
                                                                               \
    a = U2[i1] - U1[i1];                                                       \
    b = -(U2[i0] - U1[i0]);                                                    \
    c = -a * U1[i0] - b * U1[i1];                                              \
    d1 = a * V0[i0] + b * V0[i1] + c;                                          \
                                                                               \
    a = U0[i1] - U2[i1];                                                       \
    b = -(U0[i0] - U2[i0]);                                                    \
    c = -a * U2[i0] - b * U2[i1];                                              \
    d2 = a * V0[i0] + b * V0[i1] + c;                                          \
    if (d0 * d1 > 0.0) {                                                       \
      if (d0 * d2 > 0.0)                                                       \
        return 1;                                                              \
    }                                                                          \
  }

int coplanar_tri_tri(float N[3], float V0[3], float V1[3], float V2[3],
                     float U0[3], float U1[3], float U2[3]) {
  float A[3];
  short i0, i1;
  /* first project onto an axis-aligned plane, that maximizes the area */
  /* of the triangles, compute indices: i0,i1. */
  A[0] = fabs(N[0]);
  A[1] = fabs(N[1]);
  A[2] = fabs(N[2]);
  if (A[0] > A[1]) {
    if (A[0] > A[2]) {
      i0 = 1; /* A[0] is greatest */
      i1 = 2;
    } else {
      i0 = 0; /* A[2] is greatest */
      i1 = 1;
    }
  } else /* A[0]<=A[1] */
  {
    if (A[2] > A[1]) {
      i0 = 0; /* A[2] is greatest */
      i1 = 1;
    } else {
      i0 = 0; /* A[1] is greatest */
      i1 = 2;
    }
  }

  /* test all edges of triangle 1 against the edges of triangle 2 */
  EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2);
  EDGE_AGAINST_TRI_EDGES(V1, V2, U0, U1, U2);
  EDGE_AGAINST_TRI_EDGES(V2, V0, U0, U1, U2);

  /* finally, test if tri1 is totally contained in tri2 or vice versa */
  POINT_IN_TRI(V0, U0, U1, U2);
  POINT_IN_TRI(U0, V0, V1, V2);

  return 0;
}

int tri_tri_intersect(float V0[3], float V1[3], float V2[3], float U0[3],
                      float U1[3], float U2[3]) {
  float E1[3], E2[3];
  float N1[3], N2[3], d1, d2;
  float du0, du1, du2, dv0, dv1, dv2;
  float D[3];
  float isect1[2], isect2[2];
  float du0du1, du0du2, dv0dv1, dv0dv2;
  short index;
  float vp0, vp1, vp2;
  float up0, up1, up2;
  float b, c, max;

  /* compute plane equation of triangle(V0,V1,V2) */
  SUB(E1, V1, V0);
  SUB(E2, V2, V0);
  CROSS(N1, E1, E2);
  d1 = -DOT(N1, V0);
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to the
   * plane*/
  du0 = DOT(N1, U0) + d1;
  du1 = DOT(N1, U1) + d1;
  du2 = DOT(N1, U2) + d1;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST == TRUE
  if (fabs(du0) < EPSILON)
    du0 = 0.0;
  if (fabs(du1) < EPSILON)
    du1 = 0.0;
  if (fabs(du2) < EPSILON)
    du2 = 0.0;
#endif
  du0du1 = du0 * du1;
  du0du2 = du0 * du2;

  if (du0du1 > 0.0f &&
      du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;        /* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  SUB(E1, U1, U0);
  SUB(E2, U2, U0);
  CROSS(N2, E1, E2);
  d2 = -DOT(N2, U0);
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0 = DOT(N2, V0) + d2;
  dv1 = DOT(N2, V1) + d2;
  dv2 = DOT(N2, V2) + d2;

#if USE_EPSILON_TEST == TRUE
  if (fabs(dv0) < EPSILON)
    dv0 = 0.0;
  if (fabs(dv1) < EPSILON)
    dv1 = 0.0;
  if (fabs(dv2) < EPSILON)
    dv2 = 0.0;
#endif

  dv0dv1 = dv0 * dv1;
  dv0dv2 = dv0 * dv2;

  if (dv0dv1 > 0.0f &&
      dv0dv2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;        /* no intersection occurs */

  /* compute direction of intersection line */
  CROSS(D, N1, N2);

  /* compute and index to the largest component of D */
  max = fabs(D[0]);
  index = 0;
  b = fabs(D[1]);
  c = fabs(D[2]);
  if (b > max)
    max = b, index = 1;
  if (c > max)
    max = c, index = 2;

  /* this is the simplified projection onto L*/
  vp0 = V0[index];
  vp1 = V1[index];
  vp2 = V2[index];

  up0 = U0[index];
  up1 = U1[index];
  up2 = U2[index];

  /* compute interval for triangle 1 */
  COMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, isect1[0],
                    isect1[1]);

  /* compute interval for triangle 2 */
  COMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, isect2[0],
                    isect2[1]);

  SORT(isect1[0], isect1[1]);
  SORT(isect2[0], isect2[1]);

  if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
    return 0;
  return 1;
}

#define NEWCOMPUTE_INTERVALS(VV0, VV1, VV2, D0, D1, D2, D0D1, D0D2, A, B, C,   \
                             X0, X1)                                           \
  {                                                                            \
    if (D0D1 > 0.0f) {                                                         \
      /* here we know that D0D2<=0.0 */                                        \
      /* that is D0, D1 are on the same side, D2 on the other or on the plane  \
       */                                                                      \
      A = VV2;                                                                 \
      B = (VV0 - VV2) * D2;                                                    \
      C = (VV1 - VV2) * D2;                                                    \
      X0 = D2 - D0;                                                            \
      X1 = D2 - D1;                                                            \
    } else if (D0D2 > 0.0f) {                                                  \
      /* here we know that d0d1<=0.0 */                                        \
      A = VV1;                                                                 \
      B = (VV0 - VV1) * D1;                                                    \
      C = (VV2 - VV1) * D1;                                                    \
      X0 = D1 - D0;                                                            \
      X1 = D1 - D2;                                                            \
    } else if (D1 * D2 > 0.0f || D0 != 0.0f) {                                 \
      /* here we know that d0d1<=0.0 or that D0!=0.0 */                        \
      A = VV0;                                                                 \
      B = (VV1 - VV0) * D0;                                                    \
      C = (VV2 - VV0) * D0;                                                    \
      X0 = D0 - D1;                                                            \
      X1 = D0 - D2;                                                            \
    } else if (D1 != 0.0f) {                                                   \
      A = VV1;                                                                 \
      B = (VV0 - VV1) * D1;                                                    \
      C = (VV2 - VV1) * D1;                                                    \
      X0 = D1 - D0;                                                            \
      X1 = D1 - D2;                                                            \
    } else if (D2 != 0.0f) {                                                   \
      A = VV2;                                                                 \
      B = (VV0 - VV2) * D2;                                                    \
      C = (VV1 - VV2) * D2;                                                    \
      X0 = D2 - D0;                                                            \
      X1 = D2 - D1;                                                            \
    } else {                                                                   \
      /* triangles are coplanar */                                             \
      return coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2);                     \
    }                                                                          \
  }

int NoDivTriTriIsect(float V0[3], float V1[3], float V2[3], float U0[3],
                     float U1[3], float U2[3]) {
  float E1[3], E2[3];
  float N1[3], N2[3], d1, d2;
  float du0, du1, du2, dv0, dv1, dv2;
  float D[3];
  float isect1[2], isect2[2];
  float du0du1, du0du2, dv0dv1, dv0dv2;
  short index;
  float vp0, vp1, vp2;
  float up0, up1, up2;
  float bb, cc, max;
  float a, b, c, x0, x1;
  float d, e, f, y0, y1;
  float xx, yy, xxyy, tmp;

  /* compute plane equation of triangle(V0,V1,V2) */
  SUB(E1, V1, V0);
  SUB(E2, V2, V0);
  CROSS(N1, E1, E2);
  d1 = -DOT(N1, V0);
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to the
   * plane*/
  du0 = DOT(N1, U0) + d1;
  du1 = DOT(N1, U1) + d1;
  du2 = DOT(N1, U2) + d1;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST == TRUE
  if (FABS(du0) < EPSILON)
    du0 = 0.0;
  if (FABS(du1) < EPSILON)
    du1 = 0.0;
  if (FABS(du2) < EPSILON)
    du2 = 0.0;
#endif
  du0du1 = du0 * du1;
  du0du2 = du0 * du2;

  if (du0du1 > 0.0f &&
      du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;        /* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  SUB(E1, U1, U0);
  SUB(E2, U2, U0);
  CROSS(N2, E1, E2);
  d2 = -DOT(N2, U0);
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0 = DOT(N2, V0) + d2;
  dv1 = DOT(N2, V1) + d2;
  dv2 = DOT(N2, V2) + d2;

#if USE_EPSILON_TEST == TRUE
  if (FABS(dv0) < EPSILON)
    dv0 = 0.0;
  if (FABS(dv1) < EPSILON)
    dv1 = 0.0;
  if (FABS(dv2) < EPSILON)
    dv2 = 0.0;
#endif

  dv0dv1 = dv0 * dv1;
  dv0dv2 = dv0 * dv2;

  if (dv0dv1 > 0.0f &&
      dv0dv2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;        /* no intersection occurs */

  /* compute direction of intersection line */
  CROSS(D, N1, N2);

  /* compute and index to the largest component of D */
  max = (float)FABS(D[0]);
  index = 0;
  bb = (float)FABS(D[1]);
  cc = (float)FABS(D[2]);
  if (bb > max)
    max = bb, index = 1;
  if (cc > max)
    max = cc, index = 2;

  /* this is the simplified projection onto L*/
  vp0 = V0[index];
  vp1 = V1[index];
  vp2 = V2[index];

  up0 = U0[index];
  up1 = U1[index];
  up2 = U2[index];

  /* compute interval for triangle 1 */
  NEWCOMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, a, b, c,
                       x0, x1);

  /* compute interval for triangle 2 */
  NEWCOMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, d, e, f,
                       y0, y1);

  xx = x0 * x1;
  yy = y0 * y1;
  xxyy = xx * yy;

  tmp = a * xxyy;
  isect1[0] = tmp + b * x1 * yy;
  isect1[1] = tmp + c * x0 * yy;

  tmp = d * xxyy;
  isect2[0] = tmp + e * xx * y1;
  isect2[1] = tmp + f * xx * y0;

  SORT(isect1[0], isect1[1]);
  SORT(isect2[0], isect2[1]);

  if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
    return 0;
  return 1;
}

/* sort so that a<=b */
#define SORT2(a, b, smallest)                                                  \
  if (a > b) {                                                                 \
    float c;                                                                   \
    c = a;                                                                     \
    a = b;                                                                     \
    b = c;                                                                     \
    smallest = 1;                                                              \
  } else                                                                       \
    smallest = 0;

inline void isect2(float VTX0[3], float VTX1[3], float VTX2[3], float VV0,
                   float VV1, float VV2, float D0, float D1, float D2,
                   float *isect0, float *isect1, float isectpoint0[3],
                   float isectpoint1[3]) {
  float tmp = D0 / (D0 - D1);
  float diff[3];
  *isect0 = VV0 + (VV1 - VV0) * tmp;
  SUB(diff, VTX1, VTX0);
  MULT(diff, diff, tmp);
  ADD(isectpoint0, diff, VTX0);
  tmp = D0 / (D0 - D2);
  *isect1 = VV0 + (VV2 - VV0) * tmp;
  SUB(diff, VTX2, VTX0);
  MULT(diff, diff, tmp);
  ADD(isectpoint1, VTX0, diff);
}

#if 0
#define ISECT2(VTX0, VTX1, VTX2, VV0, VV1, VV2, D0, D1, D2, isect0, isect1,    \
               isectpoint0, isectpoint1)                                       \
  tmp = D0 / (D0 - D1);                                                        \
  isect0 = VV0 + (VV1 - VV0) * tmp;                                            \
  SUB(diff, VTX1, VTX0);                                                       \
  MULT(diff, diff, tmp);                                                       \
  ADD(isectpoint0, diff, VTX0);                                                \
  tmp = D0 / (D0 - D2);
/*              isect1=VV0+(VV2-VV0)*tmp;          \ */
/*              SUB(diff,VTX2,VTX0);               \     */
/*              MULT(diff,diff,tmp);               \   */
/*              ADD(isectpoint1,VTX0,diff);           */
#endif

inline int compute_intervals_isectline(float VERT0[3], float VERT1[3],
                                       float VERT2[3], float VV0, float VV1,
                                       float VV2, float D0, float D1, float D2,
                                       float D0D1, float D0D2, float *isect0,
                                       float *isect1, float isectpoint0[3],
                                       float isectpoint1[3]) {
  if (D0D1 > 0.0f) {
    /* here we know that D0D2<=0.0 */
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */
    isect2(VERT2, VERT0, VERT1, VV2, VV0, VV1, D2, D0, D1, isect0, isect1,
           isectpoint0, isectpoint1);
  } else if (D0D2 > 0.0f) {
    /* here we know that d0d1<=0.0 */
    isect2(VERT1, VERT0, VERT2, VV1, VV0, VV2, D1, D0, D2, isect0, isect1,
           isectpoint0, isectpoint1);
  } else if (D1 * D2 > 0.0f || D0 != 0.0f) {
    /* here we know that d0d1<=0.0 or that D0!=0.0 */
    isect2(VERT0, VERT1, VERT2, VV0, VV1, VV2, D0, D1, D2, isect0, isect1,
           isectpoint0, isectpoint1);
  } else if (D1 != 0.0f) {
    isect2(VERT1, VERT0, VERT2, VV1, VV0, VV2, D1, D0, D2, isect0, isect1,
           isectpoint0, isectpoint1);
  } else if (D2 != 0.0f) {
    isect2(VERT2, VERT0, VERT1, VV2, VV0, VV1, D2, D0, D1, isect0, isect1,
           isectpoint0, isectpoint1);
  } else {
    /* triangles are coplanar */
    return 1;
  }
  return 0;
}

#define COMPUTE_INTERVALS_ISECTLINE(VERT0, VERT1, VERT2, VV0, VV1, VV2, D0,    \
                                    D1, D2, D0D1, D0D2, isect0, isect1,        \
                                    isectpoint0, isectpoint1)                  \
  if (D0D1 > 0.0f) {                                                           \
    /* here we know that D0D2<=0.0 */                                          \
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
    isect2(VERT2, VERT0, VERT1, VV2, VV0, VV1, D2, D0, D1, &isect0, &isect1,   \
           isectpoint0, isectpoint1);                                          \
  }
#if 0
  else if(D0D2>0.0f)                                    \
  {                                                     \
    /* here we know that d0d1<=0.0 */                   \
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D1*D2>0.0f || D0!=0.0f)                       \
  {                                                     \
    /* here we know that d0d1<=0.0 or that D0!=0.0 */   \
    isect2(VERT0,VERT1,VERT2,VV0,VV1,VV2,D0,D1,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D1!=0.0f)                                     \
  {                                                     \
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D2!=0.0f)                                     \
  {                                                     \
    isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else                                                  \
  {                                                     \
    /* triangles are coplanar */                        \
    coplanar=1;                                         \
    return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2);      \
  }
#endif

int tri_tri_intersect_with_isectline(float V0[3], float V1[3], float V2[3],
                                     float U0[3], float U1[3], float U2[3],
                                     int *coplanar, float isectpt1[3],
                                     float isectpt2[3]) {
  float E1[3], E2[3];
  float N1[3], N2[3], d1, d2;
  float du0, du1, du2, dv0, dv1, dv2;
  float D[3];
  float isect1[2], isect2[2];
  float isectpointA1[3], isectpointA2[3];
  float isectpointB1[3], isectpointB2[3];
  float du0du1, du0du2, dv0dv1, dv0dv2;
  short index;
  float vp0, vp1, vp2;
  float up0, up1, up2;
  float b, c, max;
  float tmp, diff[3];
  int smallest1, smallest2;

  /* compute plane equation of triangle(V0,V1,V2) */
  SUB(E1, V1, V0);
  SUB(E2, V2, V0);
  CROSS(N1, E1, E2);
  d1 = -DOT(N1, V0);
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to the
   * plane*/
  du0 = DOT(N1, U0) + d1;
  du1 = DOT(N1, U1) + d1;
  du2 = DOT(N1, U2) + d1;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST == TRUE
  if (fabs(du0) < EPSILON)
    du0 = 0.0;
  if (fabs(du1) < EPSILON)
    du1 = 0.0;
  if (fabs(du2) < EPSILON)
    du2 = 0.0;
#endif
  du0du1 = du0 * du1;
  du0du2 = du0 * du2;

  if (du0du1 > 0.0f &&
      du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;        /* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  SUB(E1, U1, U0);
  SUB(E2, U2, U0);
  CROSS(N2, E1, E2);
  d2 = -DOT(N2, U0);
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0 = DOT(N2, V0) + d2;
  dv1 = DOT(N2, V1) + d2;
  dv2 = DOT(N2, V2) + d2;

#if USE_EPSILON_TEST == TRUE
  if (fabs(dv0) < EPSILON)
    dv0 = 0.0;
  if (fabs(dv1) < EPSILON)
    dv1 = 0.0;
  if (fabs(dv2) < EPSILON)
    dv2 = 0.0;
#endif

  dv0dv1 = dv0 * dv1;
  dv0dv2 = dv0 * dv2;

  if (dv0dv1 > 0.0f &&
      dv0dv2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;        /* no intersection occurs */

  /* compute direction of intersection line */
  CROSS(D, N1, N2);

  /* compute and index to the largest component of D */
  max = fabs(D[0]);
  index = 0;
  b = fabs(D[1]);
  c = fabs(D[2]);
  if (b > max)
    max = b, index = 1;
  if (c > max)
    max = c, index = 2;

  /* this is the simplified projection onto L*/
  vp0 = V0[index];
  vp1 = V1[index];
  vp2 = V2[index];

  up0 = U0[index];
  up1 = U1[index];
  up2 = U2[index];

  /* compute interval for triangle 1 */
  *coplanar = compute_intervals_isectline(
      V0, V1, V2, vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, &isect1[0],
      &isect1[1], isectpointA1, isectpointA2);
  if (*coplanar)
    return coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2);

  /* compute interval for triangle 2 */
  compute_intervals_isectline(U0, U1, U2, up0, up1, up2, du0, du1, du2, du0du1,
                              du0du2, &isect2[0], &isect2[1], isectpointB1,
                              isectpointB2);

  SORT2(isect1[0], isect1[1], smallest1);
  SORT2(isect2[0], isect2[1], smallest2);

  if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
    return 0;

  /* at this point, we know that the triangles intersect */

  if (isect2[0] < isect1[0]) {
    if (smallest1 == 0) {
      SET(isectpt1, isectpointA1);
    } else {
      SET(isectpt1, isectpointA2);
    }

    if (isect2[1] < isect1[1]) {
      if (smallest2 == 0) {
        SET(isectpt2, isectpointB2);
      } else {
        SET(isectpt2, isectpointB1);
      }
    } else {
      if (smallest1 == 0) {
        SET(isectpt2, isectpointA2);
      } else {
        SET(isectpt2, isectpointA1);
      }
    }
  } else {
    if (smallest2 == 0) {
      SET(isectpt1, isectpointB1);
    } else {
      SET(isectpt1, isectpointB2);
    }

    if (isect2[1] > isect1[1]) {
      if (smallest1 == 0) {
        SET(isectpt2, isectpointA2);
      } else {
        SET(isectpt2, isectpointA1);
      }
    } else {
      if (smallest2 == 0) {
        SET(isectpt2, isectpointB2);
      } else {
        SET(isectpt2, isectpointB1);
      }
    }
  }
  return 1;
}