#include "gif_writer.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

struct Key {
  int prefix;
  int suffix;
  bool operator==(const Key &other) const {
    return prefix == other.prefix && suffix == other.suffix;
  }
};

struct KeyHash {
  size_t operator()(const Key &key) const {
    return (static_cast<size_t>(key.prefix) << 8) ^
           static_cast<size_t>(key.suffix);
  }
};

void write_u16(std::ostream &out, int value) {
  const unsigned v = static_cast<unsigned>(value);
  const char bytes[2] = {static_cast<char>(v & 0xFF),
                         static_cast<char>((v >> 8) & 0xFF)};
  out.write(bytes, 2);
}

// GIF LZW, matching the code-size growth used by gifenc (clear on a full 12-bit table).
std::vector<uint8_t> lzw_encode(const std::vector<uint8_t> &pixels) {
  constexpr int kDegree = 256;
  const int clear_code = kDegree;
  const int stop_code = kDegree + 1;
  int nkeys = kDegree + 2;
  int key_size = 9;

  std::unordered_map<Key, int, KeyHash> dict;
  std::vector<uint8_t> bytes;
  uint32_t partial = 0;
  int bit_count = 0;

  auto write_bits = [&](int code, int size) {
    partial |= static_cast<uint32_t>(code) << bit_count;
    bit_count += size;
    while (bit_count >= 8) {
      bytes.push_back(static_cast<uint8_t>(partial & 0xFF));
      partial >>= 8;
      bit_count -= 8;
    }
  };

  write_bits(clear_code, key_size);
  if (!pixels.empty()) {
    int node = pixels[0];
    for (size_t i = 1; i < pixels.size(); ++i) {
      const int pixel = pixels[i];
      const Key key{node, pixel};
      const auto found = dict.find(key);
      if (found != dict.end()) {
        node = found->second;
        continue;
      }
      write_bits(node, key_size);
      if (nkeys < 0x1000) {
        if (nkeys == (1 << key_size))
          ++key_size;
        dict.emplace(key, nkeys++);
      } else {
        write_bits(clear_code, key_size);
        dict.clear();
        nkeys = kDegree + 2;
        key_size = 9;
      }
      node = pixel;
    }
    write_bits(node, key_size);
  }
  write_bits(stop_code, key_size);
  if (bit_count > 0)
    bytes.push_back(static_cast<uint8_t>(partial & 0xFF));
  return bytes;
}

void write_subblocks(std::ostream &out, const std::vector<uint8_t> &bytes) {
  size_t offset = 0;
  while (offset < bytes.size()) {
    const size_t n = std::min<size_t>(255, bytes.size() - offset);
    out.put(static_cast<char>(n));
    out.write(reinterpret_cast<const char *>(bytes.data() + offset),
              static_cast<std::streamsize>(n));
    offset += n;
  }
  out.put(0);
}

uint32_t pack_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | b;
}

}  // namespace

bool write_animated_gif(const std::string &path,
                        const std::vector<graphics::GraphicsContext> &frames,
                        int delay_cs, int hold_last_cs, int scale,
                        std::string &error) {
  if (frames.empty()) {
    error = "no frames to write";
    return false;
  }
  if (scale < 1)
    scale = 1;
  if (delay_cs < 2)
    delay_cs = 2;
  if (hold_last_cs < delay_cs)
    hold_last_cs = delay_cs;

  const int src_w = frames[0].get_width();
  const int src_h = frames[0].get_height();
  for (const auto &frame : frames) {
    if (frame.get_width() != src_w || frame.get_height() != src_h) {
      error = "frames differ in size";
      return false;
    }
  }

  std::vector<uint8_t> palette(256 * 3, 0);
  std::unordered_map<uint32_t, int> index_of;
  int color_count = 0;
  auto color_index = [&](uint8_t r, uint8_t g, uint8_t b) -> int {
    const uint32_t key = pack_rgb(r, g, b);
    const auto found = index_of.find(key);
    if (found != index_of.end())
      return found->second;
    if (color_count >= 256)
      return -1;
    const int index = color_count++;
    palette[index * 3] = r;
    palette[index * 3 + 1] = g;
    palette[index * 3 + 2] = b;
    index_of.emplace(key, index);
    return index;
  };

  for (const auto &frame : frames) {
    for (const graphics::Pixel &pixel : frame.get_pixels()) {
      if (color_index(pixel.r, pixel.g, pixel.b) < 0) {
        error = "timer frame uses more than 256 colors";
        return false;
      }
    }
  }

  const int width = src_w * scale;
  const int height = src_h * scale;
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    error = "could not open " + path;
    return false;
  }

  out.write("GIF89a", 6);
  write_u16(out, width);
  write_u16(out, height);
  out.put(static_cast<char>(0xF7));  // global color table, 256 entries
  out.put(0);
  out.put(0);
  out.write(reinterpret_cast<const char *>(palette.data()),
            static_cast<std::streamsize>(palette.size()));

  // Loop forever.
  const unsigned char netscape[] = {
      0x21, 0xFF, 0x0B, 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0',
      0x03, 0x01, 0x00, 0x00, 0x00};
  out.write(reinterpret_cast<const char *>(netscape), sizeof(netscape));

  std::vector<uint8_t> indices(static_cast<size_t>(width) * height);
  for (size_t frame_i = 0; frame_i < frames.size(); ++frame_i) {
    const auto &pixels = frames[frame_i].get_pixels();
    for (int y = 0; y < height; ++y) {
      const int src_y = y / scale;
      for (int x = 0; x < width; ++x) {
        const int src_x = x / scale;
        const graphics::Pixel &pixel = pixels[src_y * src_w + src_x];
        indices[static_cast<size_t>(y) * width + x] = static_cast<uint8_t>(
            color_index(pixel.r, pixel.g, pixel.b));
      }
    }

    const int delay = frame_i + 1 == frames.size() ? hold_last_cs : delay_cs;
    const unsigned char gce[] = {
        0x21, 0xF9, 0x04, 0x04,
        static_cast<unsigned char>(delay & 0xFF),
        static_cast<unsigned char>((delay >> 8) & 0xFF), 0x00, 0x00};
    out.write(reinterpret_cast<const char *>(gce), sizeof(gce));
    out.put(0x2C);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, width);
    write_u16(out, height);
    out.put(0x00);
    out.put(8);  // LZW minimum code size
    write_subblocks(out, lzw_encode(indices));
  }

  out.put(0x3B);
  if (!out) {
    error = "failed while writing " + path;
    return false;
  }
  return true;
}
