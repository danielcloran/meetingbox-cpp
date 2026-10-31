#include "graphics/graphics.hpp"
#include "graphics/sdl_renderer.hpp"

namespace graphics {

std::unique_ptr<Renderer> create_present_renderer() {
  return std::make_unique<SDL2Renderer>();
}

}  // namespace graphics
