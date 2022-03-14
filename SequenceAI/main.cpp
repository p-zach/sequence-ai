#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

#include "Game.hpp"
#include "Card.hpp"
#include "Constants.hpp"

using namespace sf;
using namespace std;

int main()
{
    // Initialize window
    RenderWindow window(VideoMode(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT), "Sequence");

    Game game;

    Clock clock;

    // Main loop
    while (window.isOpen())
    {
        // Handle screen events
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        // Clear previous frame
        window.clear();

        float elapsed = clock.restart().asSeconds();
        game.update(window, elapsed);
        game.draw(window);

        // Push drawn pixels to screen
        window.display();
    }

    return 0;
}