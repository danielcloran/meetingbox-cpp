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

    sf::RectangleShape rect(sf::Vector2f(5, 5));
    rect.setFillColor(sf::Color::Yellow);
    rect.setPosition(sf::Vector2f(10, 10));

    // sf::RectangleShape rect2(sf::Vector2f(5, 5));
    // rect2.setFillColor(sf::Color::Red);
    // rect2.setPosition(sf::Vector2f(30, 30));

    renderTexture.clear(sf::Color::Black);
    // renderTexture.draw(rect);
    // renderTexture.draw(rect2);
    // renderTexture.display();
    // TODO: Pass the framebuffer to your set_pixels function


    draw(renderTexture, width, height);

    return 0;
}
