extern "C" {
    #include "graphite.h"
}

#include "camera.h"
#include "plane.h"
#include "scene.h"

#include <ctime>
#include <cstdio>
#include <SDL.h>

void sim_run() {

    Camera camera(60.0f);

    Scene scene;

    auto plane = std::make_shared<Plane>("f22.obj", "f22.png");
    plane->m_position = {0.0f, 0.1f, -15.0f, 1.0f};
    scene.add_entity(plane);

    auto runway = std::make_shared<Entity>("runway.obj", "runway.png");
    runway->m_transform = matrix_make_translation(0.0f, -0.5f, 3.0f);
    scene.add_entity(runway);

    auto terrain = std::make_shared<Entity>("cube.obj", "cube.png");
    auto m = matrix_make_identity();
    auto s = matrix_make_scale(100.0f, 1.0f, 100.0f);
    auto t = matrix_make_translation(0.0f, -2.1f, 0.0f);
    m = matrix_multiply_matrix(&t, &m);
    m = matrix_multiply_matrix(&s, &m);
    terrain->m_transform = m;
    scene.add_entity(terrain);

    auto tower = std::make_shared<Plane>("cube.obj", "cube.png");
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

        plane->update(delta_time);
        camera.update(view, *(std::dynamic_pointer_cast<Plane>(plane).get()), {13.0f, 20.0f, 0.0f});
        plane->m_visible = view != Camera::Views::COCKPIT_FORWARD;
        tower->m_visible = view != Camera::Views::TOWER;

        clock_t t1_draw = clock();

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
            plane->m_input_forward += 0.1f;
        if (key_s)
            if (plane->m_input_forward > 0.0f)
                plane->m_input_forward -= 0.1f;
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

        clock_t t2_draw = clock();
        delta_time = (float)(t2_draw - t1_draw) / (float)CLOCKS_PER_SEC;

        frame_counter++;
        if ((frame_counter % 100) == 0)
            printf("FPS: %.1f\n", 1.0f / delta_time);
    }
}