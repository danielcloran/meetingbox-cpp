#include "graphics/graphics.hpp"
#include "graphics/rpi_renderer.hpp"

namespace graphics {

std::unique_ptr<Renderer> create_present_renderer() {
  return std::make_unique<RpiRenderer>();
}

}  // namespace graphics
