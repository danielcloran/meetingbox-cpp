#ifndef GRAPHICS_SDL_RENDERER_HPP
#define GRAPHICS_SDL_RENDERER_HPP

#include "graphics/graphics.hpp"

#include <SDL2/SDL.h>

namespace graphics {

// Demo and default-host present backend. The product path is RpiRenderer
// (//media:RpiRenderer, HUB75 / rpi-rgb-led-matrix on linux aarch64).
class SDL2Renderer : public Renderer {
 public:
  SDL2Renderer();
  ~SDL2Renderer();
  void init() override;
  bool init_present() override;
  // Hidden 192x192 software renderer for headless readback. No panel borders.
  bool init_capture();
  bool capture(const GraphicsContext &context, GraphicsContext &box, std::string &error);
  void draw(const GraphicsContext &context) override;
  void cleanup() override;
  bool should_close() override;
  bool ready() const { return renderer_ != nullptr; }

 private:
  SDL_Window *window_ = nullptr;
  SDL_Renderer *renderer_ = nullptr;
  SDL_Texture *texture_ = nullptr;
  int frame_count_ = 0;
  bool quit_ = false;
  bool draw_borders_ = true;

  bool create_renderer(bool capture);
  bool paint_to_renderer(const GraphicsContext &context);
  bool read_box(GraphicsContext &box, std::string &error);

  void draw_led_panels(const GraphicsContext &context);
  void draw_single_panel(const GraphicsContext &context, int panel_index, int x_offset);
};

}  // namespace graphics

#endif
