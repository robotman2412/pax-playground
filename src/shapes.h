
// SPDX-License-Identidier: MIT

#pragma once

#include <pax_gfx.h>

// Types of shape storage.
typedef enum {
    // A straight line.
    SHAPE_TYPE_LINE,
    // Any triangle.
    SHAPE_TYPE_TRIANGLE,
    // Any rectangle.
    SHAPE_TYPE_RECTANGLE,
    // A polygon with 3+ points.
    SHAPE_TYPE_POLYGON,
    // A circle of arbitrary granularity.
    SHAPE_TYPE_CIRCLE,
} shape_type_t;

// Valid rendering modes.
typedef enum {
    // Render normally.
    RENDERMODE_NORMAL,
    // Render outline.
    RENDERMODE_OUTLINE,
    // Render wireframe.
    RENDERMODE_WIREFRAME,
} rendermode_t;

// Definition of a single shape.
typedef struct {
    // Type of shape this is.
    shape_type_t type;
    // Current transformation matrix.
    matrix_2d_t  matrix;
    // Shape color.
    pax_col_t    color;
    // Description of the shape.
    union {
        // A straight line.
        pax_linef line;
        // Any triangle.
        pax_trif  triangle;
        // Any rectangle.
        pax_rectf rectangle;
        // A polygon with 3+ points.
        struct {
            // Number of points.
            size_t     points_len;
            // Points array.
            pax_vec2f *points;
            // UVs array.
            pax_vec2f *uvs;
        } polygon;
        // A circle of arbitrary granularity.
        struct {
            // Center point of the circle.
            pax_vec2f center;
            // Radius of the circle.
            float     radius;
        } circle;
    };
} shape_t;

// Metarendering modes.
typedef enum {
    // Don't metarender.
    METARENDER_NONE,
    // Show the perfect shape outline.
    METARENDER_OUTLINE,
    // Show the perfect triangles.
    METARENDER_TRIANGLE,
    // Show the triangles' trapezoids.
    METARENDER_TRAPEZOID,
    // Show the triangles' horizontal lines.
    METARENDER_HLINE,
} metarender_t;

// Render and metarender a shape.
void shape_renderall(pax_buf_t *gfx, pax_buf_t *meta, metarender_t meta_mode, shape_t *shape);
// Render a shape.
void shape_render(pax_buf_t *gfx, shape_t *shape) {
    shape_renderall(gfx, NULL, METARENDER_NONE, shape);
}
// Metarender a shape.
void shape_metarender(pax_buf_t *meta, metarender_t meta_mode, shape_t *shape) {
    shape_renderall(NULL, meta, meta_mode, shape);
}
