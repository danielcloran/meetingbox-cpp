#include "graphics/graphics.hpp"

namespace graphics
{
    SDL_Renderer *renderer_ = nullptr;

    namespace internal
    {
        std::atomic<bool> quit_(false);
        int process_screen_id_ = 0;
        std::map<int, ProcessScreen> process_screens_;
        SDL_Surface *screen_ = nullptr;
    }

    void initialize()
    {
        if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_VIDEO & ~SDL_INIT_AUDIO) < 0)
        {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return;
        }
        // Allow SDL2 to attempt anti-aliasing
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

        internal::screen_ = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
        if (!internal::screen_)
        {
            std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return;
        }
        renderer_ = SDL_CreateSoftwareRenderer(internal::screen_);

        // After SDL2 has been initialized, we let the platform specific renderer init
        Renderer::initialize();


        // draw rect
        // SDL_Rect rect;
        // rect.x = 20;
        // rect.y = 0;
        // rect.w = 20;
        // rect.h = 20;
        // SDL_FillRect(internal::process_screens_.at(screen2).surface, &rect, SDL_MapRGBA(internal::process_screens_.at(screen2).surface->format, 255, 0, 0, 255));


        // fill the renderer_ with blue
        // SDL_SetRenderDrawColor(renderer_, 130, 70, 140, 210);
        // SDL_RenderClear(renderer_);

        // load the timebox.png
        int screen2 = add_process_screen(Screen::ScreenType::MIMICK_ALL);
        // SDL_Texture *tex = IMG_LoadTexture(internal::process_screens_.at(screen2).renderer, "../graphics/timebox.png");
        // // copy to surface
        // SDL_RenderCopy(internal::process_screens_.at(screen2).renderer, tex, NULL, &Screen::screen_sizes_.at(internal::process_screens_.at(screen2).screen_type));


        // fill with red
        SDL_SetRenderDrawColor(internal::process_screens_.at(screen2).renderer, 220, 0, 0, 80);
        SDL_RenderClear(internal::process_screens_.at(screen2).renderer);

        // SDL_RenderCopy(internal::process_screens_.at(screen2).renderer, tex, NULL, NULL);

        // set render target
        // SDL_SetRenderTarget(renderer_, internal::process_screens_.at(screen2).textu);
        // SDL_RenderDrawLine(renderer_, 0, 0, 200, 200);
        // int screenId = add_process_screen(Screen::ScreenType::TOP);
        // SDL_Renderer *rend = get_process_renderer(screenId);
        // // load image to renderer
        // SDL_Texture *tex = IMG_LoadTexture(rend, "../graphics/timebox.png");
        // SDL_RenderCopy(rend, tex, NULL, NULL);
        // SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
        // SDL_RenderDrawLine(rend, 0, 0, 200, 200);
        // SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);

        // SDL_RenderDrawPointF(rend, 2.2, 20.8);
        // // SDL_RenderClear(rend);
    }

    void loop()
    {
        SDL_Event event;
        while (!internal::quit_.load())
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    internal::quit_.store(true);
                }
            }

            // iterate through process_screens_ backward and draw **IN ORDER**
            for (auto pair = internal::process_screens_.rbegin(); pair != internal::process_screens_.rend(); ++pair)
            {
                ProcessScreen screen = pair->second;
                switch (screen.screen_type)
                {
                case Screen::MIMICK_ALL:
                    SDL_BlitSurface(screen.surface, NULL, internal::screen_, &(Screen::screen_sizes_.at(Screen::TOP)));
                case Screen::MIMICK_SIDES:
                    for (SDL_Rect panel : Screen::sides_)
                    {
                        SDL_BlitSurface(screen.surface, NULL, internal::screen_, &panel);
                    }
                    break;
                default:
                    SDL_BlitSurface(screen.surface, NULL, internal::screen_, &(Screen::screen_sizes_.at(screen.screen_type)));
                }
            }
            Renderer::draw(internal::screen_);

            SDL_Delay(10); // let the CPU rest
        }
    }

    void quit()
    {
        internal::quit_.store(true);
        SDL_DestroyRenderer(renderer_);
        SDL_FreeSurface(internal::screen_);
    }

    int add_process_screen(Screen::ScreenType screen_type)
    {
        SDL_Surface *surface = SDL_CreateRGBSurface(0, Screen::screen_sizes_.at(screen_type).w, Screen::screen_sizes_.at(screen_type).h, 32, 0, 0, 0, 0);

        // set surface to transparent
        SDL_SetColorKey(surface, SDL_TRUE, TRANSPARENT);
        SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
        internal::process_screens_.insert(std::make_pair(internal::process_screen_id_, ProcessScreen{surface, renderer, screen_type}));
        return internal::process_screen_id_++;
    }

    SDL_Renderer *get_process_renderer(int screen_id)
    {
        SDL_Renderer *rend = internal::process_screens_.at(screen_id).renderer;

        return rend;
    }

    void remove_process_screen(int screen_id)
    {
        SDL_DestroyRenderer(internal::process_screens_.at(screen_id).renderer);
        SDL_FreeSurface(internal::process_screens_.at(screen_id).surface);
        internal::process_screens_.erase(screen_id);
    }

}
