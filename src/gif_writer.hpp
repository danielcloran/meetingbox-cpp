#ifndef GIF_WRITER_HPP
#define GIF_WRITER_HPP

#include "graphics/graphics.hpp"

#include <string>
#include <vector>

// Write a looping GIF of box frames. Frames are scaled with nearest-neighbor
// so the 192x192 LED cross stays blocky. delay_cs is in centiseconds.
bool write_animated_gif(const std::string &path,
                        const std::vector<graphics::GraphicsContext> &frames,
                        int delay_cs, int hold_last_cs, int scale,
                        std::string &error);

#endif
