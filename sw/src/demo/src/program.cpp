#include "common.h"

#include "camera.h"
#include "plane.h"
#include "scene.h"

#include <cstring>
#include <cstdlib>
#include <vector>

int nb_triangles;
bool rasterizer_ena = true;


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

    graphite_init();

    Camera camera(60.0f);

    Scene scene;

    auto plane = std::make_shared<Plane>("/assets/f22.obj", "/assets/f22.png");
    plane->m_position = {0.0f, 0.1f, -15.0f, 1.0f};
    scene.add_entity(plane);

    auto runway = std::make_shared<Entity>("/assets/runway.obj", "/assets/runway.png");
    runway->m_transform = matrix_make_translation(0.0f, -0.5f, 3.0f);
    scene.add_entity(runway);

    auto terrain = std::make_shared<Entity>("/assets/cube.obj", "/assets/cube.png");
    auto m = matrix_make_identity();
    auto s = matrix_make_scale(100.0f, 1.0f, 100.0f);
    auto t = matrix_make_translation(0.0f, -2.1f, 0.0f);
    m = matrix_multiply_matrix(&t, &m);
    m = matrix_multiply_matrix(&s, &m);
    terrain->m_transform = m;
    scene.add_entity(terrain);

    auto tower = std::make_shared<Plane>("/assets/cube.obj", "/assets/cube.png");
    m = matrix_make_identity();
    s = matrix_make_scale(3.0f, 10.0f, 3.0f);
    t = matrix_make_translation(10.0f, 10.0f, 0.0f);
    m = matrix_multiply_matrix(&t, &m);
    m = matrix_multiply_matrix(&s, &m);
    tower->m_transform = m;
    scene.add_entity(tower);


    bool quit = false;
    bool print_stats = false;
    bool is_textured = true;
    size_t nb_lights = 1;
    bool clamp_s = false;
    bool clamp_t = false;
    bool perspective_correct = true;
    bool gouraud_shading = true;

    light_t lights[5];
    lights[0].direction = (vec3d){0.0f, -1.0f, 0.0f, 0.0f};
    lights[0].ambient_color = (vec3d){0.1f, 0.1f, 0.1f, 1.0f};
    lights[0].diffuse_color = (vec3d){1.0f, 1.0f, 1.0f, 1.0f};
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

    bool key_up = false;
    bool key_down = false;
    bool key_left = false;
    bool key_right = false;
    bool key_w = false;
    bool key_s = false;
    bool key_a = false;
    bool key_d = false;

    float delta_time = 0.0f;
    int frame_counter = 0;

    Camera::Views view = Camera::Views::COCKPIT_FORWARD;

    while(!quit) {

        MEM_WRITE(LED, frame_counter >> 2);

        plane->update(delta_time);
        camera.update(view, *(std::dynamic_pointer_cast<Plane>(plane).get()), {13.0f, 20.0f, 0.0f});
        plane->m_visible = view != Camera::Views::COCKPIT_FORWARD;

        uint32_t t1_draw = MEM_READ(TIMER);

        clear(0x31A6);

        camera.begin_drawing();
        scene.draw(&camera, lights, nb_lights);
        camera.end_drawing();

        swap();

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
                        case SDLK_a:
                            key_a = true;
                            break;
                        case SDLK_d:
                            key_d = true;
                            break;
                        case SDLK_1:
                            view = Camera::Views::COCKPIT_FORWARD;
                            break;
                        case SDLK_2:
                            view = Camera::Views::COCKPIT_LEFT;
                            break;
                        case SDLK_3:
                            view = Camera::Views::COCKPIT_RIGHT;
                            break;
                        case SDLK_4:
                            view = Camera::Views::FOLLOW;
                            break;
                        case SDLK_5:
                            view = Camera::Views::TOWER;
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
                        case SDLK_a:
                            key_a = false;
                            break;
                        case SDLK_d:
                            key_d = false;
                            break;
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                    }
                    break;
            }
        }

        if (key_w)
            plane->m_input_forward += 0.2f;
        if (key_s)
            if (plane->m_input_forward > 0.0f)
                plane->m_input_forward -= 0.2f;
        if (key_left)
            plane->m_input_roll = 0.5f;
        else if (key_right)
            plane->m_input_roll = -0.5f;
        else
            plane->m_input_roll = 0.0f;
        if (key_up)
            plane->m_input_pitch = 0.3f;
        else if (key_down)
            plane->m_input_pitch = -0.3f;
        else
            plane->m_input_pitch = 0.0f;
        if (key_a)
            plane->m_input_yaw = -0.5f;
        else if (key_d)
            plane->m_input_yaw = 0.5;
        else
            plane->m_input_yaw = 0.0f;

        uint32_t t2_draw = MEM_READ(TIMER);
        delta_time = (t2_draw - t1_draw) / 1000.0f;

        frame_counter++;
        if ((frame_counter % 5) == 0)
            printf("FPS: %.1f\n", 1.0f / delta_time);
    }

    fl_shutdown();    

    return 0;
}
