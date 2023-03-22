#include "draw.hpp"

void draw(sf::RenderTexture &renderTexture, int width, int height) {
    sf::RenderWindow window(sf::VideoMode(width, height), "SFML Window");

    // Create sprite from texture
    const sf::Texture& texture = renderTexture.getTexture();
    sf::Sprite sprite(texture);

    // Draw sprite on window
    window.draw(sprite);

    // Display window
    window.display();

    // Main loop
    while (window.isOpen())
    {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
    }
}