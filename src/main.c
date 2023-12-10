
// SPDX-License-Identidier: MIT

#include "scenes.h"

#include <pax_gfx.h>
#include <SDL.h>

// SDL window.
SDL_Window   *window;
// SDL renderer.
SDL_Renderer *renderer;
// PAX graphics context.
pax_buf_t     gfx;
// Current scene.
int           scene;

// Flush the contents of a buffer to the window.
void window_flush(SDL_Window *window, pax_buf_t *gfx) {
    static SDL_Texture *texture = NULL;
    static int          tw, th;
    SDL_Surface        *surface = SDL_GetWindowSurface(window);

    if (surface->w == pax_buf_get_width(gfx) && surface->h == pax_buf_get_height(gfx)) {
        // Direct surface update.
        SDL_LockSurface(surface);
        memcpy(surface->pixels, pax_buf_get_pixels(gfx), pax_buf_get_size(gfx));
        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);

    } else {
        // Texture based update.
        if (texture && (tw != pax_buf_get_width(gfx) || th != pax_buf_get_height(gfx))) {
            SDL_DestroyTexture(texture);
            texture = NULL;
        }
        if (!texture) {
            tw = pax_buf_get_width(gfx);
            th = pax_buf_get_height(gfx);
            SDL_Texture *texture =
                SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, tw, th);
        }
        if (texture) {
            SDL_Renderer *renderer = SDL_GetRenderer(window);
            void         *pixeldata;
            int           pitch;
            SDL_LockTexture(texture, NULL, &pixeldata, &pitch);
            memcpy(pixeldata, pax_buf_get_pixels(gfx), pax_buf_get_size(gfx));
            SDL_UnlockTexture(texture);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }
}

// Update the graphics.
void update_gfx(pax_buf_t *gfx);

int main(int argc, char **argv) {
    // Create the SDL contexts.
    SDL_Init(SDL_INIT_VIDEO);
    int res = SDL_CreateWindowAndRenderer(400, 300, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_SetWindowTitle(window, "PAX playground");

    pax_buf_init(&gfx, NULL, 400, 300, PAX_BUF_32_8888ARGB);
    update_gfx(&gfx);
    window_flush(window, &gfx);

    bool      lctrl = false;
    SDL_Event event;
    while (1) {
        if (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    pax_buf_destroy(&gfx);
                    int width, height;
                    SDL_GetWindowSize(window, &width, &height);
                    pax_buf_init(&gfx, NULL, width, height, PAX_BUF_32_8888ARGB);
                    update_gfx(&gfx);
                    window_flush(window, &gfx);
                }
            }
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                if (event.key.keysym.sym == SDLK_LCTRL)
                    lctrl = true;
                if (event.key.keysym.sym == SDLK_q && lctrl)
                    break;
                if (event.key.keysym.sym == SDLK_LEFT) {
                    scene = (scene + SCENE_COUNT - 1) % SCENE_COUNT;
                    update_gfx(&gfx);
                    window_flush(window, &gfx);
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    scene = (scene + 1) % SCENE_COUNT;
                    update_gfx(&gfx);
                    window_flush(window, &gfx);
                }
            }
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_LCTRL)
                    lctrl = false;
            }
        } else {
            break;
        }
    }

    return 0;
}



// Pseudorandom direction vector.
unsigned pix_rand(int ix, int iy) {
    unsigned const w = 8 * sizeof(unsigned);
    unsigned const s = w / 2;
    unsigned       a = ix, b = iy;
    a *= 3284157443;
    b ^= a << s | a >> w - s;
    b *= 1911520717;
    a ^= b << s | b >> w - s;
    a *= 2048419325;
    return a;
}
// Shader that does noise.
pax_col_t shader_noise_func(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    int n = pix_rand(x / 4, y / 4) % 8 + 16;
    return pax_col_rgb(n, n, n);
}
pax_shader_t shader_noise = {
    .schema_version    = 1,
    .schema_complement = ~1,
    .callback          = shader_noise_func,
    .promise_callback  = NULL,
    .alpha_promise_0   = false,
    .alpha_promise_255 = true,
};

// Shader that interpolates alpha
pax_col_t shader_alpha_func(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    return pax_col_lerp((u + v) * (tint >> 25), existing, 0xff000000 | tint);
}
pax_shader_t shader_alpha = {
    .schema_version    = 1,
    .schema_complement = ~1,
    .callback          = shader_alpha_func,
    .promise_callback  = NULL,
    .alpha_promise_0   = false,
    .alpha_promise_255 = false,
};

// Shader that shows UV coordinates.
pax_col_t shader_uv_func(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    return pax_col_rgb(u * 255, v * 255, 0);
}
pax_shader_t shader_uv = {
    .schema_version    = 1,
    .schema_complement = ~1,
    .callback          = shader_uv_func,
    .promise_callback  = NULL,
    .alpha_promise_0   = false,
    .alpha_promise_255 = true,
};

// Shader that does rainbow colors.
pax_col_t shader_hsv_func(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    return pax_col_hsv(u * 255, 255 - v * 255, 255);
}
pax_shader_t shader_hsv = {
    .schema_version    = 1,
    .schema_complement = ~1,
    .callback          = shader_hsv_func,
    .promise_callback  = NULL,
    .alpha_promise_0   = false,
    .alpha_promise_255 = true,
};

// Graphics demo: Shaders.
void demo_shaders(pax_buf_t *gfx) {
    int width  = pax_buf_get_width(gfx);
    int height = pax_buf_get_height(gfx);
    int min    = width < height ? width : height;
    pax_shade_rect(gfx, -1, &shader_noise, NULL, 0, 0, width, height);

    int   n         = 3;
    float scale     = fminf(width / (float)n * 0.8, height * 0.8);
    float spacing   = (width - n * scale) / (n + 1);
    float offset    = spacing + scale / 2;
    float increment = spacing + scale;

    pax_apply_2d(gfx, matrix_2d_translate(offset, height / 2.0f));
    pax_shade_rect(gfx, 0x7f0000ff, &shader_alpha, NULL, -scale / 2, -scale / 2, scale, scale);
    pax_apply_2d(gfx, matrix_2d_translate(increment, 0));
    pax_shade_rect(gfx, -1, &shader_uv, NULL, -scale / 2, -scale / 2, scale, scale);
    pax_apply_2d(gfx, matrix_2d_translate(increment, 0));
    pax_shade_rect(gfx, -1, &shader_hsv, NULL, -scale / 2, -scale / 2, scale, scale);
}



// Graphics demo: Rasterization.
void demo_rasterization(pax_buf_t *gfx) {
    int width  = pax_buf_get_width(gfx);
    int height = pax_buf_get_height(gfx);
    int min    = width < height ? width : height;
    pax_background(gfx, 0xff000000);

    pax_vec2f tri[] = {{0.6, 0.9}, {5.1, 17.6}, {18.2, 6.3}};
    int       sw = 20, sh = 20;
    int       scale = (min - 40) / sw;

    pax_draw_tri(gfx, 0xffff0000, tri[0].x, tri[0].y, tri[1].x, tri[1].y, tri[2].x, tri[2].y);
    pax_join();

    // Enlarge the triangle.
    pax_apply_2d(gfx, matrix_2d_translate((width - scale * sw) / 2, (height - scale * sh) / 2));
    pax_apply_2d(gfx, matrix_2d_scale(scale, scale));
    pax_shader_t shader = {
        .schema_version    = 1,
        .schema_complement = (uint8_t)~1,
        .renderer_id       = PAX_RENDERER_ID_SWR,
        .promise_callback  = NULL,
        .callback          = (void *)pax_shader_texture,
        .callback_args     = (void *)(gfx),
        .alpha_promise_0   = true,
        .alpha_promise_255 = true

    };
    pax_quadf uvs = {
        0,
        0,
        (float)sw / (float)width,
        0,
        (float)sw / (float)width,
        (float)sh / (float)height,
        0,
        (float)sh / (float)height,
    };
    pax_shade_rect(gfx, -1, &shader, &uvs, 0, 0, sw, sh);
    for (int i = 0; i <= sw; i++) {
        pax_draw_line(gfx, 0x3f000000, i, 0, i, sh);
        pax_draw_line(gfx, 0x3f000000, 0, i, sw, i);
    }
    pax_outline_tri(gfx, 0xffffffff, tri[0].x, tri[0].y, tri[1].x, tri[1].y, tri[2].x, tri[2].y);
}



// Update the graphics.
void update_gfx(pax_buf_t *gfx) {
    pax_reset_2d(gfx, true);
    switch (scene) {
        case SCENE_RASTERIZATION: demo_rasterization(gfx); break;
        case SCENE_SHADERS: demo_shaders(gfx); break;
    }
}
