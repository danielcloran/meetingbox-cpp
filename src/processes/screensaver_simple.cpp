#include "processes/screensaver.hpp"
#include "graphics/graphics.hpp"

ScreensaverProcess::ScreensaverProcess(int processId, int screenId, Json::Value info) : Process(processId, screenId, info) {
    defaultSaver = ScreensaverTypes::LOADING;
}

static void SolidColor()
{
    // Simple solid color screensaver
    graphics::display_message("Screensaver");
}

static void RotatingColors(long long timeElapsed)
{
    // Simple rotating colors effect
    static int continuum = 0;
    continuum %= 3 * 255;
    
    // Calculate color values for rotating colors effect
    int r = (continuum < 255) ? 255 - continuum : (continuum < 2 * 255) ? 0 : continuum - 2 * 255;
    int g = (continuum < 255) ? continuum : (continuum < 2 * 255) ? 2 * 255 - continuum : 0;
    int b = (continuum < 255) ? 0 : (continuum < 2 * 255) ? continuum - 255 : 3 * 255 - continuum;
    
    // Use the color values in the display (suppress unused variable warnings)
    (void)r; (void)g; (void)b;
    
    // Display a simple pattern
    graphics::display_message("Colors");
}

static void Loading(long long timeElapsed)
{
    // Simple loading animation
    graphics::display_loading();
}

void ScreensaverProcess::draw(long long timeElapsed)
{
    // Updated to use simple graphics instead of SDL2
    switch (defaultSaver)
    {
    case ScreensaverTypes::SOLID_COLOR:
        SolidColor();
        break;
    case ScreensaverTypes::ROTATING_COLORS:
        RotatingColors(timeElapsed);
        break;
    case ScreensaverTypes::LOADING:
        Loading(timeElapsed);
        break;
    default:
        graphics::display_message("Unknown");
        break;
    }
}
