#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <atomic>

// LED Matrix dimensions. Five 64x64 panels in one horizontal strip.
const int LED_WIDTH = 320;
const int LED_HEIGHT = 64;
const int LED_PANELS = 5;  // 5 separate 64x64 panels

namespace graphics
{
    // Simple RGB pixel structure
    struct Pixel
    {
        uint8_t r, g, b;
        constexpr Pixel(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
            : r(red), g(green), b(blue) {}
        constexpr bool operator==(const Pixel& other) const
        {
            return r == other.r && g == other.g && b == other.b;
        }
        constexpr bool operator!=(const Pixel& other) const
        {
            return !(*this == other);
        }
    };

    // Unfolded box: a 3x3 grid of panels. Four corners stay kBoxBackground.
    constexpr int kPanelSize = 64;
    constexpr int kBoxWidth = kPanelSize * 3;
    constexpr int kBoxHeight = kPanelSize * 3;
    constexpr Pixel kBoxBackground{0x10, 0x10, 0x10};

    struct BoxPanelPlacement
    {
        int strip_index;
        int dest_x;
        int dest_y;
    };
    constexpr int kBoxPanelCount = 5;
    const BoxPanelPlacement* box_panel_placements();
    
    // Graphics context for drawing operations
    class GraphicsContext
    {
    public:
        GraphicsContext(int width, int height);
        ~GraphicsContext() = default;
        
        // Drawing operations
        void clear(const Pixel& color = Pixel(0,0,0));
        void set_pixel(int x, int y, const Pixel& color);
        void draw_text(const std::string& text, int x, int y, const Pixel& color);
        void draw_rectangle(int x, int y, int width, int height, const Pixel& color);
        void draw_circle(int x, int y, int radius, const Pixel& color);
        
        // Get pixel data for LED matrix
        const std::vector<Pixel>& get_pixels() const { return pixels_; }
        int get_width() const { return width_; }
        int get_height() const { return height_; }
        
        // Save to PPM file (for debugging). Returns false if the file cannot be opened.
        bool save_to_ppm(const std::string& filename) const;
        
    private:
        std::vector<Pixel> pixels_;
        int width_;
        int height_;
        
        bool is_valid_position(int x, int y) const;
    };

    // Copy the 320x64 strip onto the 192x192 unfolded box.
    // Strip: 0 left, 1 front, 2 top (lid, middle panel), 3 right, 4 back.
    // Cross center is the lid. See box_layout.cpp.
    void paint_box_layout(const GraphicsContext& strip, GraphicsContext& box);

    // Solid panel colours plus labels, matching graphics_test.
    void paint_graphics_test_pattern(GraphicsContext& strip);

    // SDL software-renderer readback of the unfolded box.
    // Uses SDL_VIDEODRIVER=dummy when that variable is unset.
    // Defined by //media:SdlRenderer (demo readback, not the Pi present path).
    bool capture_painted_box(const GraphicsContext& strip, GraphicsContext& box, std::string& error);

    // Present backend. Bazel picks the concrete type:
    //   //media:RpiRenderer — Raspberry Pi aarch64 + HUB75 rpi-rgb-led-matrix
    //   //media:SdlRenderer — SDL window or dummy driver (demo / default host)
    // //media:present is that select. WS281x is not a present backend.
    // The MCP tools do not change.
    class Renderer
    {
    public:
        virtual ~Renderer() = default;
        virtual void init() = 0;
        // Present thread. SDL opens a hidden dummy-driver window.
        // Pi initializes the HUB75 matrix. False: skip draw and keep the last frame.
        virtual bool init_present() = 0;
        virtual void draw(const GraphicsContext& context) = 0;
        virtual void cleanup() = 0;
        virtual bool should_close() = 0;
    };

    // Defined by the one present factory //media:graphics links
    // (present_rpi.cpp or present_sdl.cpp).
    std::unique_ptr<Renderer> create_present_renderer();

    // Main graphics interface
    extern std::unique_ptr<Renderer> renderer_;
    extern std::unique_ptr<GraphicsContext> context_;
    extern std::atomic<bool> quit_;
    
    void initialize();
    void loop();
    void quit();
    
    // Simplified interface for processes
    void display_message(const std::string& message);
    void display_loading();
    void display_timer(int minutes, int seconds);
}

#endif // GRAPHICS_HPP