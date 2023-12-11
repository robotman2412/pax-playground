
// SPDX-License-Identidier: MIT

#include "shapes.h"

#include <pax_gfx.h>

// Contrast shader.
pax_col_t shader_contrast_func(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    uint16_t sum = (tint & 255) + ((tint & 255) >> 8) + ((tint & 255) >> 16);
    return pax_col_lerp(tint >> 24, existing, sum >= 184 ? 0xff000000 : 0xffffffff);
}
pax_shader_t shader_contrast = {
    .schema_version    = 1,
    .schema_complement = ~1,
    .callback          = shader_contrast_func,
    .promise_callback  = NULL,
    .alpha_promise_0   = false,
    .alpha_promise_255 = true,
};

// Metarender a trapezoid.
static void
    metarender_trapezoid(pax_buf_t *meta, bool hlines, float x0a, float x0b, float y0, float x1a, float x1b, float y1) {
    // Sort points vertically.
    if (y1 < y0) {
        float tmp;
        tmp = x1a;
        x1a = x0a;
        x0a = tmp;
        tmp = x1b;
        x1b = x0b;
        x0b = tmp;
        tmp = y1;
        y1  = y0;
        y0  = tmp;
    }

    // Align the pixels.
    int iy0 = y0 + 0.5;
    int iy1 = y1 + 0.5;
    if (iy0 >= iy1)
        return;

    float ay0 = iy0 + 0.5;
    float ay1 = iy1 + 0.5;
    x0a       = x0a + (x1a - x0a) * (ay0 - y0) / (y1 - y0);
    x0b       = x0b + (x1b - x0b) * (ay0 - y0) / (y1 - y0);
    y0        = ay0;
    x1a       = x0a + (x1a - x0a) * (ay1 - y0) / (y1 - y0);
    x1b       = x0b + (x1b - x0b) * (ay1 - y0) / (y1 - y0);
    y1        = ay1;

    if (hlines) {
        float dxa = (x1a - x0a) / (y1 - y0);
        float dxb = (x1b - x0b) / (y1 - y0);
        for (float y = y0, xa = x0a, xb = x0b; y < y1; y++, xa += dxa, xb += dxb) {
            pax_shade_line(meta, -1, &shader_contrast, NULL, xa, y - 0.3, xa, y + 0.3);
            pax_shade_line(meta, -1, &shader_contrast, NULL, xb, y - 0.3, xb, y + 0.3);
            pax_shade_line(meta, -1, &shader_contrast, NULL, xa, y, xb, y);
        }
    } else {
        pax_shade_line(meta, -1, &shader_contrast, NULL, x0a, y0, x0b, y0);
        pax_shade_line(meta, -1, &shader_contrast, NULL, x1a, y1, x1b, y1);
        pax_shade_line(meta, -1, &shader_contrast, NULL, x0a, y0, x1a, y1);
        pax_shade_line(meta, -1, &shader_contrast, NULL, x0b, y0, x1b, y1);
    }
}

// Render a triangle.
static void render_tri(pax_buf_t *gfx, pax_buf_t *meta, metarender_t meta_mode, pax_tri_t shape, pax_col_t color) {
    if (gfx) {
        pax_draw_tri(gfx, color, shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2);
    }
    if (meta && (meta_mode == METARENDER_OUTLINE || meta_mode == METARENDER_TRIANGLE)) {
        pax_shade_outline_tri(
            meta,
            -1,
            &shader_contrast,
            NULL,
            shape.x0,
            shape.y0,
            shape.x1,
            shape.y1,
            shape.x2,
            shape.y2
        );

    } else if (meta && (meta_mode == METARENDER_TRAPEZOID || meta_mode == METARENDER_HLINE)) {
        float interp = shape.x0 + (shape.x2 - shape.x0) * (shape.y1 - shape.y0) / (shape.y2 - shape.y0);
        metarender_trapezoid(
            meta,
            meta_mode == METARENDER_HLINE,
            shape.x0,
            shape.x0,
            shape.y0,
            shape.x1,
            interp,
            shape.y1
        );
    }
}

// Render a rectangle.
static void render_rect(pax_buf_t *gfx, pax_buf_t *meta, metarender_t meta_mode, pax_rect_t shape, pax_col_t color) {
}

// Render and metarender a shape.
void shape_renderall(pax_buf_t *gfx, pax_buf_t *meta, metarender_t meta_mode, shape_t *shape) {
}
