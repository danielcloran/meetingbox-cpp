#include "graphics/graphics.hpp"
#include "graphics/draw.hpp"

void run()
{
    if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_VIDEO & ~SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Surface *surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
    if (!surface)
    {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
    init();

    IMG_Init(IMG_INIT_PNG);
    // SDL_Surface * image = IMG_Load("../godot_64x64.png");
    SDL_Texture *texture = IMG_LoadTexture(renderer, "../graphics/godot_64x64.png");


    // SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 100, 0));

    // SDL_Rect square = {100, 7, 50, 50};
    // SDL_FillRect(surface, &square, SDL_MapRGB(surface->format, 80, 0, 0));

    // // Do something with pixelData, e.g., send it to the LED panel using SPI
    // // Draw surface to window

    // draw(surface);
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    bool quit = false;
    SDL_Event event;
    float angle = 0.0f;

    const std::vector<SDL_Vertex> verts =
        {
            {
                SDL_FPoint{0, 0},
                SDL_Color{255, 0, 0, 255},
                SDL_FPoint{0},
            },
            {
                SDL_FPoint{20, 45},
                SDL_Color{0, 0, 255, 255},
                SDL_FPoint{0},
            },
            {
                SDL_FPoint{60, 45},
                SDL_Color{0, 255, 0, 255},
                SDL_FPoint{0},
            },
        };

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 20, 0, 0));
        // SDL_RenderGeometry(renderer, nullptr, verts.data(), verts.size(), nullptr, 0);

        // int squareWidth = 20;
        // int squareHeight = 20;
        // int centerX = WIDTH / 2;
        // int centerY = HEIGHT / 2;
        // int offsetX = squareWidth / 2;
        // int offsetY = squareHeight / 2;

        // float radians = angle * M_PI / 180.0f;
        // int xOffset = static_cast<int>(offsetX * cos(radians) - offsetY * sin(radians));
        // int yOffset = static_cast<int>(offsetX * sin(radians) + offsetY * cos(radians));

        // SDL_Rect square = {centerX - xOffset, centerY - yOffset, squareWidth, squareHeight};
        // SDL_FillRect(surface, &square, SDL_MapRGB(surface->format, 80, 0, 0));

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        draw(surface);

        // angle += 0.2f;
        // if (angle >= 360.0f)
        // {
        //     angle = 0.0f;
        // }

        // SDL_Delay(20);
    }

    SDL_FreeSurface(surface);
    IMG_Quit();
    SDL_Quit();
}
