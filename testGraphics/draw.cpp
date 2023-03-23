#include "draw.hpp"

void init_window(sf::RenderWindow &window) {
    window.create(sf::VideoMode(3200, 640), "My Window");
    window.setVisible(true);
}

void draw(sf::RenderWindow &window, sf::RenderTexture &renderTexture, int width, int height)
{
    sf::RenderStates states;
    states.transform.translate(0, 0);
    sf::Sprite sprite(renderTexture.getTexture());
    sprite.setScale(10.f, 10.f);
    window.draw(sprite, states);

    // Display the window
    window.display();

    // Wait for the user to close the window
    while (window.isOpen())
    {
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