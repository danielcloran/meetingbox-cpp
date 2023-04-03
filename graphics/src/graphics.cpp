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
}

void Graphics::start()
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
        SDL_Texture *texture = IMG_LoadTexture(renderer_, "../graphics/godot_64x64.png");
        SDL_RenderCopy(renderer_, texture, NULL, &(Screen::screen_sizes_.at(Screen::TOP)));

        // iterate through process_screens_
        for (const auto &pair : process_screens_)
        {
            ProcessScreen value = pair.second;

            // Possibly quite inefficient
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, value.surface);
            SDL_RenderCopy(renderer_, tex, NULL, &(Screen::screen_sizes_.at(value.screen_type)));
            SDL_DestroyTexture(tex);
        }
        Renderer::draw(screen_);
    }
}

void Graphics::quit()
{
    quit_ = true;
    // SDL_DestroyRenderer(renderer_);
    // SDL_FreeSurface(screen_);
    // SDL_Quit();
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
