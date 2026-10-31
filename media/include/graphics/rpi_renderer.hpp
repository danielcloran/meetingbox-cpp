#ifndef GRAPHICS_RPI_RENDERER_HPP
#define GRAPHICS_RPI_RENDERER_HPP

#include "graphics/graphics.hpp"

namespace graphics {

// Product present backend: HUB75 panels via rpi-rgb-led-matrix.
// Bazel target //media:RpiRenderer. Not the demo host.
class RpiRenderer : public Renderer {
 public:
  RpiRenderer();
  ~RpiRenderer() override;
  void init() override;
  bool init_present() override;
  void draw(const GraphicsContext &context) override;
  void cleanup() override;
  bool should_close() override { return false; }

 private:
  struct Hub75;
  Hub75 *hub_ = nullptr;
  bool ready_ = false;
};

}  // namespace graphics

#endif
