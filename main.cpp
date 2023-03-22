#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

#include "draw.hpp"


int main()
{
    const int width = 64;
    const int height = 64;

    sf::RenderTexture renderTexture;
    renderTexture.create(width, height);

    sf::RectangleShape rect(sf::Vector2f(32.f, 32.f));
    rect.setFillColor(sf::Color::Red);
    rect.setPosition(sf::Vector2f(16.f, 16.f));

    renderTexture.clear(sf::Color::Black);
    renderTexture.draw(rect);
    renderTexture.display();
    // TODO: Pass the framebuffer to your set_pixels function


    draw(renderTexture, width, height);

    return 0;
}
