#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <atomic>
#include <SDL.h>

// LED Matrix dimensions
const int LED_WIDTH = 320;
const int LED_HEIGHT = 64;
const int LED_PANELS = 5;  // 5 separate 64x64 panels

namespace graphics
{
    // Simple RGB pixel structure
    struct Pixel
    {
        uint8_t r, g, b;
        Pixel(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) 
            : r(red), g(green), b(blue) {}
    };
    
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
        
        // Save to PPM file (for debugging)
        void save_to_ppm(const std::string& filename) const;
        
    private:
        std::vector<Pixel> pixels_;
        int width_;
        int height_;
        
        bool is_valid_position(int x, int y) const;
    };
    
    // Platform-specific renderer
    class Renderer
    {
    public:
        virtual ~Renderer() = default;
        virtual void init() = 0;
        virtual void draw(const GraphicsContext& context) = 0;
        virtual void cleanup() = 0;
        virtual bool should_close() = 0;
    };
    
    // SDL2 window renderer for Mac development
    class SDL2Renderer : public Renderer
    {
    public:
        SDL2Renderer();
        ~SDL2Renderer();
        void init() override;
        void draw(const GraphicsContext& context) override;
        void cleanup() override;
        bool should_close() override;

    private:
        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;
        SDL_Texture* texture_ = nullptr;
        int frame_count_ = 0;
        bool quit_ = false;
        
        void draw_led_panels(const GraphicsContext& context);
        void draw_single_panel(const GraphicsContext& context, int panel_index, int x_offset);
    };
    
    // Raspberry Pi LED matrix renderer
    class RpiRenderer : public Renderer
    {
    public:
        RpiRenderer();
        ~RpiRenderer();
        void init() override;
        void draw(const GraphicsContext& context) override;
        void cleanup() override;
        bool should_close() override { return false; }  // RPi doesn't have window close
        
    private:
        void* matrix_ __attribute__((unused)) = nullptr;  // RGBMatrix* - will be properly typed when we include the header
    };
    
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