#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

#include "draw.hpp"


int main()
{
    const int width = 3200;
    const int height = 640;

    sf::RenderWindow window;
    init_window(window);

    sf::RenderTexture renderTexture;
    renderTexture.create(64, 64);


    sf::RectangleShape rect(sf::Vector2f(30, 30));
    rect.setFillColor(sf::Color::Yellow);
    rect.setPosition(sf::Vector2f(10, 10));

    // sf::RectangleShape rect2(sf::Vector2f(5, 5));
    // rect2.setFillColor(sf::Color::Red);
    // rect2.setPosition(sf::Vector2f(30, 30));

    renderTexture.clear(sf::Color::Black);
    renderTexture.draw(rect);
    // renderTexture.draw(rect2);
    // renderTexture.display();
    // TODO: Pass the framebuffer to your set_pixels function


    draw(window, renderTexture, 64, 64);

    return 0;
}

// #include <SFML/Graphics.hpp>

// // function to slowly fade between colors of rainbow using RGBA and then convert to sf::Color
// sf::Color rainbow(float t)
// {
//     float r = 0.0f;
//     float g = 0.0f;
//     float b = 0.0f;
//     float a = 1.0f;

//     if (t < 0.25f)
//     {
//         r = 1.0f;
//         g = t * 4.0f;
//     }
//     else if (t < 0.5f)
//     {
//         r = 1.0f - (t - 0.25f) * 4.0f;
//         g = 1.0f;
//     }
//     else if (t < 0.75f)
//     {
//         g = 1.0f;
//         b = (t - 0.5f) * 4.0f;
//     }
//     else
//     {
//         g = 1.0f - (t - 0.75f) * 4.0f;
//         b = 1.0f;
//     }

//     return sf::Color(r * 255, g * 255, b * 255, a * 255);
// }





// int main()
// {
//     // Create a 64x64 window
//     sf::RenderWindow window(sf::VideoMode(200, 200), "Rainbow Triangle");

//     // Create a triangle shape
//     sf::ConvexShape triangle;
//     triangle.setPointCount(4);
//     triangle.setPoint(0, sf::Vector2f(50, 50));
//     triangle.setPoint(1, sf::Vector2f(50, 100));
//     triangle.setPoint(2, sf::Vector2f(100, 100));
//     triangle.setPoint(3, sf::Vector2f(100, 50));
//     triangle.setOrigin(75, 75);
//     triangle.setPosition(64, 64);

//     // Create a rainbow color array
//     sf::Color colors[] = {sf::Color::Red, sf::Color::Yellow, sf::Color::Green, sf::Color::Cyan, sf::Color::Blue, sf::Color::Magenta};

//     // Set the initial color of the triangle
//     triangle.setFillColor(colors[0]);

//     // Set the initial rotation of the triangle
//     float rotation = 0.0f;
//     triangle.setRotation(rotation);

//     // Start the game loop
//     while (window.isOpen())
//     {
//         // Handle events
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed)
//             {
//                 window.close();
//             }
//         }

//         // Rotate the triangle
//         rotation += 0.01f;
//         triangle.setRotation(rotation);

//         // Change the color of the triangle
//         // int colorIndex = static_cast<int>(rotation) % 6;
//         triangle.setFillColor(rainbow(rotation));

//         // Clear the window
//         window.clear(sf::Color::White);

//         // Draw the triangle to the window
//         window.draw(triangle);

//         // Display the window
//         window.display();
//     }

//     return 0;
// }