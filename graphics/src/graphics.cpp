#include "graphics/graphics.hpp"

void Graphics::initialize()
{
    if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_VIDEO & ~SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }
    // Allow SDL2 to attempt anti-aliasing
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    screen_ = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
    if (!screen_)
    {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }
    renderer_ = SDL_CreateSoftwareRenderer(screen_);

    // After SDL2 has been initialized, we let the platform specific renderer init
    Renderer::initialize();

    // Start the graphics paint thread
    quit_ = false;
    process_screen_id_ = 0;

    int screen2 = add_process_screen(Screen::ScreenType::TOP);
    // draw rect
    SDL_Rect rect;
    rect.x = 20;
    rect.y = 0;
    rect.w = 20;
    rect.h = 20;
    SDL_FillRect(process_screens_.at(screen2).surface, &rect, SDL_MapRGBA(process_screens_.at(screen2).surface->format, 255, 0, 0, 255));

    int screenId = add_process_screen(Screen::ScreenType::TOP);
    SDL_Surface *image = IMG_Load("../graphics/timebox.png");
    SDL_BlitSurface(image, NULL, process_screens_.at(screenId).surface, NULL);



}

int Graphics::add_process_screen(Screen::ScreenType screen_type)
{
    SDL_Surface *surface = SDL_CreateRGBSurface(0, Screen::screen_sizes_.at(screen_type).w, Screen::screen_sizes_.at(screen_type).h, 32, 0, 0, 0, 0);
    // set surface to transparent
    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 0, 0, 0, 128));
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGBA(surface->format, 0, 0, 0, 128));

    process_screens_.insert(std::make_pair(process_screen_id_, ProcessScreen{surface, screen_type}));
    return process_screen_id_++;
}

void Graphics::remove_process_screen(int screen_id)
{
    SDL_FreeSurface(process_screens_.at(screen_id).surface);
    process_screens_.erase(screen_id);
}

void Graphics::process()
{
    SDL_Event event;
    while (!quit_)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit_ = true;
            }
        }

        // iterate through process_screens_ backward and draw **IN ORDER**
        for (auto pair = process_screens_.rbegin(); pair != process_screens_.rend(); ++pair)
        {
            ProcessScreen screen = pair->second;
            switch (screen.screen_type)
            {
            case Screen::MIMICK_ALL:
                SDL_BlitSurface(screen.surface, NULL, screen_, &(Screen::screen_sizes_.at(Screen::TOP)));
            case Screen::MIMICK_SIDES:
                for (SDL_Rect panel : Screen::sides_)
                {
                    SDL_BlitSurface(screen.surface, NULL, screen_, &panel);
                }
                break;
            default:
                SDL_BlitSurface(screen.surface, NULL, screen_, &(Screen::screen_sizes_.at(screen.screen_type)));
            }
        }
        Renderer::draw(screen_);
    }
}

void Graphics::quit()
{
    quit_ = true;
    SDL_DestroyRenderer(renderer_);
    SDL_FreeSurface(screen_);
}

// void run()
// {
//     init();

//     IMG_Init(IMG_INIT_PNG);
//     // SDL_Surface * image = IMG_Load("../godot_64x64.png");
//     SDL_Texture *texture = IMG_LoadTexture(renderer, "../graphics/godot_64x64.png");
//     IMG_Animation *animation = IMG_LoadAnimation("../graphics/smoke.gif");
//     int current_frame = 0;

//     // SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 100, 0));

//     // Create 5 64x64 screens on the 320x64 surface
//     std::vector<SDL_Rect> screens = {
//         {0, 0, 64, 64},
//         {64, 0, 64, 64},
//         {128, 0, 64, 64},
//         {192, 0, 64, 64},
//         {256, 0, 64, 64},
//     };

//     // SDL_Rect square = {100, 7, 50, 50};
//     // SDL_FillRect(surface, &square, SDL_MapRGB(surface->format, 80, 0, 0));

//     // // Do something with pixelData, e.g., send it to the LED panel using SPI
//     // // Draw surface to window

//     // draw(surface);

//     const std::vector<SDL_Vertex> verts =
//         {
//             {
//                 SDL_FPoint{0, 0},
//                 SDL_Color{255, 0, 0, 255},
//                 SDL_FPoint{0},
//             },
//             {
//                 SDL_FPoint{20, 45},
//                 SDL_Color{0, 0, 255, 255},
//                 SDL_FPoint{0},
//             },
//             {
//                 SDL_FPoint{60, 45},
//                 SDL_Color{0, 255, 0, 255},
//                 SDL_FPoint{0},
//             },
//         };

//     while (!quit)
//     {
//         while (SDL_PollEvent(&event))
//         {
//             if (event.type == SDL_QUIT)
//             {
//                 quit = true;
//             }
//         }

//         // SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 20, 0, 0));
//         // SDL_RenderGeometry(renderer, nullptr, verts.data(), verts.size(), nullptr, 0);

//         // Iterate through each frame in the animation

//         SDL_Surface *frame = animation->frames[current_frame];
//         int delay = animation->delays[current_frame++];
//         SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, frame);
//         for (auto screen : screens)
//         {
//             SDL_RenderCopy(renderer, texture, NULL, &screen);
//         }

//         if (current_frame == animation->count)
//         {
//             current_frame = 0;
//         }

//         draw(surface);

//         SDL_Delay(delay);

//         // use animation
//         // SDL_RenderCopy(renderer, animation->texture, &animation->frames[animation->currentFrame], nullptr);

//         // draw(surface);

//         // SDL_Delay(20);
//     }

//     SDL_FreeSurface(surface);
//     IMG_Quit();
//     SDL_Quit();
// }
