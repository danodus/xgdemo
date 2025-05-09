#include "common.h"

#include <cstring>
#include <cstdlib>
#include <vector>

#define BASE_VIDEO 0x1000000

#define OP_SET_X0 0
#define OP_SET_Y0 1
#define OP_SET_Z0 2
#define OP_SET_X1 3
#define OP_SET_Y1 4
#define OP_SET_Z1 5
#define OP_SET_X2 6
#define OP_SET_Y2 7
#define OP_SET_Z2 8
#define OP_SET_R0 9
#define OP_SET_G0 10
#define OP_SET_B0 11
#define OP_SET_R1 12
#define OP_SET_G1 13
#define OP_SET_B1 14
#define OP_SET_R2 15
#define OP_SET_G2 16
#define OP_SET_B2 17
#define OP_SET_S0 18
#define OP_SET_T0 19
#define OP_SET_S1 20
#define OP_SET_T1 21
#define OP_SET_S2 22
#define OP_SET_T2 23
#define OP_CLEAR 24
#define OP_DRAW 25
#define OP_SWAP 26
#define OP_SET_TEX_ADDR 27
#define OP_SET_FB_ADDR 28

#define MEM_WRITE(_addr_, _value_) (*((volatile unsigned int *)(_addr_)) = _value_)
#define MEM_READ(_addr_) *((volatile unsigned int *)(_addr_))

#define PARAM(x) (_FLOAT_TO_FIXED(x, 14))

struct Command {
    uint32_t opcode : 8;
    uint32_t param : 24;
};

int fb_width, fb_height;

int nb_triangles;
bool rasterizer_ena = true;

void send_command(struct Command *cmd)
{
    while (!MEM_READ(GRAPHITE));
    MEM_WRITE(GRAPHITE, (cmd->opcode << 24) | cmd->param);
}

void xd_draw_triangle(vec3d p[3], vec2d t[3], vec3d c[3], texture_t* tex, bool clamp_s, bool clamp_t, int texture_scale_x, int texture_scale_y,
                      bool depth_test, bool perspective_correct)                      
{
    nb_triangles++;
    if (!rasterizer_ena)
        return;

    struct Command cmd;

    cmd.opcode = OP_SET_X0;
    cmd.param = PARAM(p[0].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[0].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Y0;
    cmd.param = PARAM(p[0].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[0].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Z0;
    cmd.param = PARAM(t[0].w) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[0].w) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_X1;
    cmd.param = PARAM(p[1].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[1].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Y1;
    cmd.param = PARAM(p[1].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[1].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Z1;
    cmd.param = PARAM(t[1].w) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[1].w) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_X2;
    cmd.param = PARAM(p[2].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[2].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Y2;
    cmd.param = PARAM(p[2].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[2].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_Z2;
    cmd.param = PARAM(t[2].w) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(p[2].z) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_S0;
    cmd.param = PARAM(t[0].u) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[0].u) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_T0;
    cmd.param = PARAM(t[0].v) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[0].v) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_S1;
    cmd.param = PARAM(t[1].u) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[1].u) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_T1;
    cmd.param = PARAM(t[1].v) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[1].v) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_S2;
    cmd.param = PARAM(t[2].u) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[2].u) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_T2;
    cmd.param = PARAM(t[2].v) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(t[2].v) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_R0;
    cmd.param = PARAM(c[0].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[0].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_G0;
    cmd.param = PARAM(c[0].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[0].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_B0;
    cmd.param = PARAM(c[0].z) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[0].z) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_R1;
    cmd.param = PARAM(c[1].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[1].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_G1;
    cmd.param = PARAM(c[1].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[1].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_B1;
    cmd.param = PARAM(c[1].z) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[1].z) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_R2;
    cmd.param = PARAM(c[2].x) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[2].x) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_G2;
    cmd.param = PARAM(c[2].y) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[2].y) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_SET_B2;
    cmd.param = PARAM(c[2].z) & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (PARAM(c[2].z) >> 16);
    send_command(&cmd);

    cmd.opcode = OP_DRAW;

    cmd.param = (depth_test ? 0b01000 : 0b00000) | (clamp_s ? 0b00100 : 0b00000) | (clamp_t ? 0b00010 : 0b00000) |
              ((tex != NULL) ? 0b00001 : 0b00000) | (perspective_correct ? 0b10000 : 0xb00000);

    cmd.param |= texture_scale_x << 5;
    cmd.param |= texture_scale_y << 8;

    send_command(&cmd);
}



void clear(unsigned int color)
{
    struct Command cmd;

    // Clear framebuffer
    cmd.opcode = OP_CLEAR;
    cmd.param = color;
    send_command(&cmd);
    // Clear depth buffer
    cmd.opcode = OP_CLEAR;
    cmd.param = 0x010000;
    send_command(&cmd);
}

void swap()
{
    struct Command cmd;

    cmd.opcode = OP_SWAP;
    cmd.param = 0x1;
    send_command(&cmd);
}


#define SDLK_SCANCODE_MASK (1<<30)
#define SDL_SCANCODE_TO_KEYCODE(x) (x | SDLK_SCANCODE_MASK)

typedef enum {
    SDL_SCANCODE_RIGHT = 79,
    SDL_SCANCODE_LEFT = 80,
    SDL_SCANCODE_DOWN = 81,
    SDL_SCANCODE_UP = 82
} SDL_Scancode;

enum {
    SDLK_UNKNOWN = 0,
    SDLK_ESCAPE = '\033',
    SDLK_1 = '1',
    SDLK_2 = '2',
    SDLK_3 = '3',
    SDLK_4 = '4',
    SDLK_5 = '5',
    SDLK_6 = '6',
    SDLK_a = 'a',
    SDLK_c = 'c',
    SDLK_d = 'd',
    SDLK_s = 's',
    SDLK_w = 'w',
    SDLK_x = 'x',

    SDLK_RIGHT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
    SDLK_LEFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
    SDLK_DOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
    SDLK_UP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP)
};

typedef enum {
    SDL_FIRSTEVENT = 0,
    SDL_KEYDOWN = 0x300,
    SDL_KEYUP
} SDL_EventType;

typedef int32_t SDL_Keycode;

typedef struct SDL_Keysym {
    SDL_Keycode sym;
} SDL_Keysym;

typedef struct {
    SDL_Keysym keysym;
} SDL_KeyboardEvent;

typedef struct {
    SDL_EventType type;
    SDL_KeyboardEvent key;
} SDL_Event;

int key_avail()
{
    unsigned int ks = MEM_READ(PS2_KBD_STATUS);
    return (ks & 0x1);
}

int get_key()
{
    while (!key_avail());
    return MEM_READ(PS2_KBD_DATA);
}

static SDL_Keycode get_keycode(int scancode, bool extended_key) {
    if (extended_key) {
        switch (scancode) {
            case 0x75:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP);
            case 0x6B:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT);
            case 0x72:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN);
            case 0x74:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT);
            default:
                return SDLK_UNKNOWN;
        }
    } else {
        switch (scancode) {
            case 0x76:
                return SDLK_ESCAPE;
            case 0x16:
                return SDLK_1;
            case 0x1E:
                return SDLK_2;
            case 0x26:
                return SDLK_3;
            case 0x25:
                return SDLK_4;
            case 0x2E:
                return SDLK_5;
            case 0x36:
                return SDLK_6;
            case 0x1C:
                return SDLK_a;
            case 0x21:
                return SDLK_c;
            case 0x23:
                return SDLK_d;
            case 0x1B:
                return SDLK_s;
            case 0x1D:
                return SDLK_w;
            case 0x22:
                return SDLK_x;
            default:
                return SDLK_UNKNOWN;
        }
    }
}

int SDL_PollEvent(SDL_Event * event) {
    event->type = SDL_FIRSTEVENT;
    if (key_avail()) {
        bool extended_key = false;
        int scancode = get_key();
        if (scancode == 0xE0) {
            // extended key
            extended_key = true;
            scancode = get_key();
        }
        if (scancode == 0xF0) {
            event->type = SDL_KEYUP;
            scancode = get_key();
            event->key.keysym.sym = get_keycode(scancode, extended_key);
        } else {
            event->type = SDL_KEYDOWN;
            event->key.keysym.sym = get_keycode(scancode, extended_key);
        }
        return 1;
    }
    return 0;
}

int read_sector(uint32_t sector, uint8_t *buffer, uint32_t sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_read_single_block(sector + i, buffer))
            return 0;
        buffer += SD_BLOCK_LEN;
    }
    return 1;
}

int write_sector(uint32_t sector, uint8_t *buffer, uint32_t sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_write_single_block(sector + i, buffer))
            return 0;
        buffer += SD_BLOCK_LEN;
    }
    return 1;
}

bool load_texture(const char *path, int *texture_scale_x, int *texture_scale_y) {
    uint32_t tex_addr = (0x1000000 >> 1) + 3 * fb_width * fb_height;

    upng_t* png_image = upng_new_from_file(path);
    if (png_image != NULL) {
        upng_decode(png_image);
        if (upng_get_error(png_image) != UPNG_EOK) {
            printf("Unable to open %s\r\n", path);
            return false;
        }
    }

    struct Command cmd;
    cmd.opcode = OP_SET_TEX_ADDR;
    cmd.param = tex_addr & 0xFFFF;
    send_command(&cmd);
    cmd.param = 0x10000 | (tex_addr >> 16);
    send_command(&cmd);

    int texture_width = upng_get_width(png_image);
    int texture_height = upng_get_height(png_image);

    uint32_t* texture_buffer = (uint32_t *)upng_get_buffer(png_image);

    uint16_t* vram = 0;
    for (int t = 0; t < texture_height; ++t)
        for (int s = 0; s < texture_width; ++s) {

            uint8_t* tc = (uint8_t*)(&texture_buffer[(texture_width * t) + s]);
            uint16_t cr = tc[0] >> 4;
            uint16_t cg = tc[1] >> 4;
            uint16_t cb = tc[2] >> 4;
            uint16_t ca = tc[3] >> 4;

            vram[tex_addr++] = (ca << 12) | (cr << 8) | (cg << 4) | cb;
        }

    upng_free(png_image);

    *texture_scale_x = -5;
    while (texture_width >>= 1) (*texture_scale_x)++;

    *texture_scale_y = -5;
    while (texture_height >>= 1) (*texture_scale_y)++;

    if (*texture_scale_x < 0 || *texture_scale_y < 0) {
        printf("Invalid texture size\r\n");
        return false;
    }

    return true;
}

static bool load_mesh_obj_data(mesh_t *mesh, const char *obj_filename) {
    FL_FILE* file;
    file = (FL_FILE *)fl_fopen(obj_filename, "r");

    if (file == NULL) {
        printf("Unable to open %s\r\n", obj_filename);
        return false;
    }

    memset(mesh, 0, sizeof(mesh_t));

    char line[1024];
    while (fl_fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            vec3d v = {
                .x = x,
                .y = y,
                .z = z,
                .w = 1.0f
            };
            mesh->vertices.emplace_back(v);
        }
        // Vertex normal information
        if (strncmp(line, "vn ", 3) == 0) {
            float x, y, z;
            sscanf(line, "vn %f %f %f", &x, &y, &z);
            vec3d v = {
                .x = x,
                .y = y,
                .z = z,
                .w = 0.0f
            };
            mesh->normals.emplace_back(v);
        }        
        // Texture coordinate information
        if (strncmp(line, "vt ", 3) == 0) {
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            vec2d t = {
                .u = u,
                .v = 1.0f - v,
                .w = 1.0f
            };
            mesh->texcoords.emplace_back(t);
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf(line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0],
                &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]
            );
            face_t face = {0};
            for (int i = 0; i < 3; i++) {
                face.indices[i] = vertex_indices[i] - 1;
                face.col_indices[i] = 0;
                face.tex_indices[i] = texture_indices[i] - 1;
                face.norm_indices[i] = normal_indices[i] - 1;
            }
            mesh->faces.emplace_back(face);
        }
    }

    fl_fclose(file);
    return true;
}

int main(void)
{
    if (!sd_init()) {
        printf("SD card initialization failed.\r\n");
        return 1;
    }

    fl_init();

    // Attach media access functions to library
    if (fl_attach_media(read_sector, write_sector) != FAT_INIT_OK)
    {
        printf("Failed to init file system\r\n");
        return 1;
    }    


    unsigned int res = MEM_READ(CONFIG);
    fb_width = res >> 16;
    fb_height = res & 0xffff;

    mat4x4 mat_proj = matrix_make_projection(fb_width, fb_height, 60.0f);

    // camera
    mat4x4 mat_view   = matrix_make_identity();

    model_t f22_model;

    if (!load_mesh_obj_data(&f22_model.mesh, "/assets/f22.obj")) {
        fl_shutdown();
        return 1;
    }

    int texture_scale_x, texture_scale_y;
    if (!load_texture("/assets/f22.png", &texture_scale_x, &texture_scale_y)) {
        fl_shutdown();
        return 1;
    }

    printf("texture scale x,y: %d, %d\r\n", texture_scale_x, texture_scale_y);

    f22_model.triangles_to_raster = (triangle_t *)malloc(2 * f22_model.mesh.faces.size() * sizeof(triangle_t));

    model_t *model = &f22_model;


    bool quit = false;
    bool print_stats = false;
    bool is_textured = true;
    size_t nb_lights = 0;
    bool clamp_s = false;
    bool clamp_t = false;
    bool perspective_correct = true;
    bool gouraud_shading = true;

    light_t lights[5];
    lights[0].direction = (vec3d){0.0f, 0.0f, 1.0f, 0.0f};
    lights[0].ambient_color = (vec3d){0.1f, 0.1f, 0.1f, 1.0f};
    lights[0].diffuse_color = (vec3d){0.5f, 0.5f, 0.5f, 1.0f};
    lights[1].direction = (vec3d){1.0f, 0.0f, 0.0f, 0.0f};
    lights[1].ambient_color = (vec3d){0.1f, 0.0f, 0.0f, 1.0f};
    lights[1].diffuse_color = (vec3d){0.2f, 0.0f, 0.0f, 1.0f};
    lights[2].direction = (vec3d){0.0f, 1.0f, 0.0f, 0.0f};
    lights[2].ambient_color = (vec3d){0.0f, 0.1f, 0.0f, 1.0f};
    lights[2].diffuse_color = (vec3d){0.0f, 0.2f, 0.0f, 1.0f};
    lights[3].direction = (vec3d){0.0f, -1.0f, 0.0f, 0.0f};
    lights[3].ambient_color = (vec3d){0.0f, 0.0f, 0.1f, 1.0f};
    lights[3].diffuse_color = (vec3d){0.0f, 0.0f, 0.2f, 1.0f};
    lights[4].direction = (vec3d){-1.0f, 0.0f, 0.0f, 0.0f};
    lights[4].ambient_color = (vec3d){0.1f, 0.1f, 0.0f, 1.0f};
    lights[4].diffuse_color = (vec3d){0.2f, 0.2f, 0.0f, 1.0f};      

    clear(0x31A6);

    uint32_t counter = 0;

    float yaw = 0.0f, pitch = 0.0f;
    vec3d vec_up = {0.0f, 1.0f, 0.0f, 1.0f};
    vec3d vec_camera = {0.0f, 0.0f, 0.0f, 1.0f};

    bool key_up = false;
    bool key_down = false;
    bool key_left = false;
    bool key_right = false;
    bool key_w = false;
    bool key_s = false;

    while(!quit) {
        MEM_WRITE(LED, counter >> 2);
        counter++;

        clear(0x31A6);

        vec3d vec_target = {0.0f, 0.0f, 1.0f, 1.0f};
        mat4x4 mat_camera_rot_y = matrix_make_rotation_y(yaw);
        mat4x4 mat_camera_rot_x = matrix_make_rotation_x(pitch);
        mat4x4 mat_camera_rot = matrix_multiply_matrix(&mat_camera_rot_x, &mat_camera_rot_y);
        vec3d vec_look_dir = matrix_multiply_vector(&mat_camera_rot, &vec_target);
        vec_target = vector_add(&vec_camera, &vec_look_dir);

        mat4x4 mat_camera = matrix_point_at(&vec_camera, &vec_target, &vec_up);

        // make view matrix from camera
        mat4x4 mat_view = matrix_quick_inverse(&mat_camera);

        //
        // world
        //

        mat4x4 mat_rot_z = matrix_make_rotation_z(0.0f);
        mat4x4 mat_rot_x = matrix_make_rotation_x(0.0f);

        mat4x4 mat_trans = matrix_make_translation(0.0f, 0.0f, 3.0f);
        mat4x4 mat_world, mat_normal;
        mat_world = matrix_make_identity();
        mat_world = mat_normal = matrix_multiply_matrix(&mat_rot_z, &mat_rot_x);
        mat_world = matrix_multiply_matrix(&mat_world, &mat_trans);

        texture_t dummy_texture;
        nb_triangles = 0;
        draw_model(fb_width, fb_height, &vec_camera, model, &mat_world, gouraud_shading ? &mat_normal : NULL, &mat_proj, &mat_view, lights, nb_lights, is_textured ? &dummy_texture : NULL, clamp_s, clamp_t, texture_scale_x, texture_scale_y, perspective_correct);
        uint32_t t2_draw = MEM_READ(TIMER);

        swap();

        vec3d vec_forward = vector_mul(&vec_look_dir, 0.1f);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            key_up = true;
                            break;
                        case SDLK_DOWN:
                            key_down = true;
                            break;
                        case SDLK_LEFT:
                            key_left = true;
                            break;
                        case SDLK_RIGHT:
                            key_right = true;
                            break;
                        case SDLK_w:
                            key_w = true;
                            break;
                        case SDLK_s:
                            key_s = true;
                            break;

                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            key_up = false;
                            break;
                        case SDLK_DOWN:
                            key_down = false;
                            break;
                        case SDLK_LEFT:
                            key_left = false;
                            break;
                        case SDLK_RIGHT:
                            key_right = false;
                            break;
                        case SDLK_w:
                            key_w = false;
                            break;
                        case SDLK_s:
                            key_s = false;
                            break;
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                    }
                    break;
            }
        }

        if (key_w)
            vec_camera = vector_add(&vec_camera, &vec_forward);
        if (key_s)
            vec_camera = vector_sub(&vec_camera, &vec_forward);
        if (key_left)
            yaw -= 0.1f;
        if (key_right)
            yaw += 0.1f;
        if (key_up)
            pitch += 0.1f;
        if (key_down)
            pitch -= 0.1f;
    }

    fl_shutdown();    

    return 0;
}
