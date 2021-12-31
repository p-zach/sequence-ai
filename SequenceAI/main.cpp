#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

#include "Card.hpp"
#include "Constants.hpp"

using namespace sf;
using namespace std;

int main()
{
    // Initialize window
    RenderWindow window(VideoMode(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT), "Sequence");

    Texture cardSheet;
    if (!cardSheet.loadFromFile("cards.png"))
    {
        cout << "Error loading card sheet. Aborting." << endl;
        exit(1);
    }
    cardSheet.setSmooth(true);

    Texture cardBack;
    if (!cardBack.loadFromFile("card_back.jpg"))
    {
        cout << "Error loading card back texture. Aborting." << endl;
        exit(1);
    }
    cardBack.setSmooth(true);

    Sprite cards[constants::NUM_SUITS][constants::NUM_FACES];

    for (int suit = 0; suit < constants::NUM_SUITS; suit++)
    {
        for (int face = 0; face < constants::NUM_FACES; face++)
        {
            cards[suit][face].setTexture(cardSheet);
            cards[suit][face].setTextureRect(Card::getCardBoundsOnBoard(suit, face));
        }
    }

    RectangleShape background(Vector2f(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT));
    background.setFillColor(Color(0, 115, 0));

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

        window.draw(background);

        Vector2f offset(constants::CARD_OFFSET_X, constants::CARD_OFFSET_Y);
        for (int y = 0; y < constants::GAME_BOARD_SIZE; y++)
        {
            for (int x = 0; x < constants::GAME_BOARD_SIZE; x++)
            {
                int cardID = constants::GAME_BOARD[y][x];
                int suit = cardID / constants::NUM_FACES;
                int face = cardID % constants::NUM_FACES;

                Sprite card;
                if (suit != constants::NUM_SUITS)
                    card = cards[suit][face];
                else
                {
                    card.setTexture(cardBack);
                }
                card.setScale(constants::DRAWN_CARD_SCALE_FACTOR, constants::DRAWN_CARD_SCALE_FACTOR);

                Vector2f position(x * (constants::CARD_WIDTH + constants::CARD_GAP), y * (constants::CARD_HEIGHT + constants::CARD_GAP));
                card.setPosition(position * constants::DRAWN_CARD_SCALE_FACTOR + offset);
                window.draw(card);
            }
        }

        // Push drawn pixels to screen
        window.display();
    }

    return 0;
}