#include "graphics/sdl_renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace graphics {

SDL2Renderer::SDL2Renderer()
    : window_(nullptr), renderer_(nullptr), texture_(nullptr), frame_count_(0),
      quit_(false) {}

SDL2Renderer::~SDL2Renderer() { cleanup(); }

void SDL2Renderer::init() {
  if (!create_renderer(/*capture=*/false)) {
    std::cerr << "SDL2 renderer failed to initialize." << std::endl;
  }
}

bool SDL2Renderer::init_present() { return init_capture(); }

bool SDL2Renderer::init_capture() {
  if (!std::getenv("SDL_VIDEODRIVER")) {
    setenv("SDL_VIDEODRIVER", "dummy", 0);
  }
  return create_renderer(/*capture=*/true);
}

bool SDL2Renderer::create_renderer(bool capture) {
  // stderr: a stdio MCP server owns stdout.
  std::cerr << "Initializing SDL2 renderer ("
            << (capture ? "headless" : "window") << ")..." << std::endl;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  // Cross of 64x64 panels. The lid is the center, not the top of the page.
  //   . B .
  //   L T R
  //   . F .
  draw_borders_ = !capture;
  const int scale = capture ? 1 : 3;
  const Uint32 window_flags = capture ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;
  window_ = SDL_CreateWindow("MeetingBox LED Matrix (Dev)",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             kBoxWidth * scale, kBoxHeight * scale, window_flags);
  if (!window_) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  const Uint32 renderer_flags =
      capture ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_ACCELERATED;
  renderer_ = SDL_CreateRenderer(window_, -1, renderer_flags);
  if (!renderer_ && !capture) {
    std::cerr << "Accelerated renderer unavailable (" << SDL_GetError()
              << "), falling back to software." << std::endl;
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
  }
  if (!renderer_) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
  if (!capture) {
    // Window is scaled up; drawing stays in 192x192 LED coordinates.
    SDL_RenderSetLogicalSize(renderer_, kBoxWidth, kBoxHeight);
    SDL_RenderSetIntegerScale(renderer_, SDL_TRUE);
  }

  std::cerr << "SDL2 renderer initialized successfully." << std::endl;
  return true;
}

bool SDL2Renderer::paint_to_renderer(const GraphicsContext &context) {
  if (!renderer_)
    return false;

  GraphicsContext box(kBoxWidth, kBoxHeight);
  paint_box_layout(context, box);

  SDL_SetRenderDrawColor(renderer_, kBoxBackground.r, kBoxBackground.g,
                         kBoxBackground.b, 0xFF);
  SDL_RenderClear(renderer_);

  const auto &pixels = box.get_pixels();
  for (int y = 0; y < kBoxHeight; ++y) {
    for (int x = 0; x < kBoxWidth; ++x) {
      const Pixel &pixel = pixels[y * kBoxWidth + x];
      SDL_SetRenderDrawColor(renderer_, pixel.r, pixel.g, pixel.b, 0xFF);
      SDL_RenderDrawPoint(renderer_, x, y);
    }
  }

  if (draw_borders_) {
    SDL_SetRenderDrawColor(renderer_, 0x40, 0x40, 0x40, 0xFF);
    const BoxPanelPlacement *panels = box_panel_placements();
    for (int i = 0; i < kBoxPanelCount; ++i) {
      SDL_Rect border = {panels[i].dest_x, panels[i].dest_y, kPanelSize,
                         kPanelSize};
      SDL_RenderDrawRect(renderer_, &border);
    }
  }
  return true;
}

void SDL2Renderer::draw(const GraphicsContext &context) {
  if (!paint_to_renderer(context))
    return;
  SDL_RenderPresent(renderer_);
  frame_count_++;
}

bool SDL2Renderer::read_box(GraphicsContext &box, std::string &error) {
  if (!renderer_) {
    error = "SDL renderer is not initialized";
    return false;
  }
  if (box.get_width() != kBoxWidth || box.get_height() != kBoxHeight) {
    error = "capture buffer must be 192x192";
    return false;
  }

  int output_w = 0;
  int output_h = 0;
  if (SDL_GetRendererOutputSize(renderer_, &output_w, &output_h) != 0) {
    error = std::string("SDL_GetRendererOutputSize failed: ") + SDL_GetError();
    return false;
  }
  if (output_w != kBoxWidth || output_h != kBoxHeight) {
    error = "SDL renderer output is " + std::to_string(output_w) + "x" +
            std::to_string(output_h) + ", expected 192x192";
    return false;
  }

  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
      0, kBoxWidth, kBoxHeight, 24, SDL_PIXELFORMAT_RGB24);
  if (!surface) {
    error = std::string("SDL_CreateRGBSurfaceWithFormat failed: ") +
            SDL_GetError();
    return false;
  }

  if (SDL_RenderReadPixels(renderer_, nullptr, SDL_PIXELFORMAT_RGB24,
                           surface->pixels, surface->pitch) != 0) {
    error = std::string("SDL_RenderReadPixels failed: ") + SDL_GetError();
    SDL_FreeSurface(surface);
    return false;
  }

  const auto *bytes = static_cast<const uint8_t *>(surface->pixels);
  for (int y = 0; y < kBoxHeight; ++y) {
    const uint8_t *row = bytes + static_cast<size_t>(y) * surface->pitch;
    for (int x = 0; x < kBoxWidth; ++x) {
      const uint8_t *pixel = row + x * 3;
      box.set_pixel(x, y, Pixel(pixel[0], pixel[1], pixel[2]));
    }
  }
  SDL_FreeSurface(surface);
  return true;
}

bool SDL2Renderer::capture(const GraphicsContext &context, GraphicsContext &box,
                           std::string &error) {
  if (!paint_to_renderer(context)) {
    error = "SDL renderer is not initialized";
    return false;
  }
  return read_box(box, error);
}

bool capture_painted_box(const GraphicsContext &strip, GraphicsContext &box,
                         std::string &error) {
  SDL2Renderer renderer;
  if (!renderer.init_capture()) {
    error = std::string("SDL2 headless init failed: ") + SDL_GetError();
    return false;
  }
  return renderer.capture(strip, box, error);
}

void SDL2Renderer::cleanup() {
  std::cerr << "Cleaning up SDL2 renderer..." << std::endl;
  if (texture_) {
    SDL_DestroyTexture(texture_);
    texture_ = nullptr;
  }
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
  SDL_Quit();
}

bool SDL2Renderer::should_close() {
  if (!window_)
    return quit_;

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      quit_ = true;
    }
  }
  return quit_;
}

void SDL2Renderer::draw_led_panels(const GraphicsContext &context) {}
void SDL2Renderer::draw_single_panel(const GraphicsContext &context,
                                     int panel_index, int x_offset) {}

}  // namespace graphics
