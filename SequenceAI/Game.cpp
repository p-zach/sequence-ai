#include "Game.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace sf;

Game::Game()
{
    loadContent();

    srand(time(0));
    reset();
}

void Game::loadTexture(Texture& texture, string file)
{
    if (!texture.loadFromFile(file))
    {
        cout << "Error loading " << file << ". Aborting." << endl;
        exit(EXIT_FAILURE);
    }
    texture.setSmooth(true);
}

void Game::loadContent()
{
    loadTexture(cardSheet, "cards.png");
    loadTexture(cardBack, "card_back.jpg");
    loadTexture(tokens[constants::P1], "token_blue.png");
    loadTexture(tokens[constants::P2], "token_red.png");

    background = RectangleShape(Vector2f(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT));
    background.setFillColor(Color(0, 115, 0));

    for (int suit = 0; suit < constants::NUM_SUITS; suit++)
    {
        for (int face = 0; face < constants::NUM_FACES; face++)
        {
            cards[suit][face].setTexture(cardSheet);
            cards[suit][face].setTextureRect(Card::getCardTextureBounds(suit, face));
        }
    }
}

void Game::reset()
{
    state = GameState::TURN_P1;

    // Initialize deck
    for (int i = 0; i < constants::DECK_SIZE; i++)
    {
        deck.push_back(Card((i % 52) / constants::NUM_FACES, (i % 52) % constants::NUM_FACES));
    }
    // Shuffle deck
    random_shuffle(deck.begin(), deck.end());

    for (int p = 0; p < constants::NUM_PLAYERS; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            hands[p][i] = deck.back();
            deck.pop_back();
        }
    }
}

IntRect Game::getHandRect(int player, int index)
{
    Vector2i size = Vector2i(constants::CARD_WIDTH, constants::CARD_HEIGHT);

    Vector2i position;
    position.x = constants::HAND_OFFSET_X + (size.x + constants::HAND_SPACING) * index;
    if (player == constants::P1)
        position.y = constants::SCREEN_HEIGHT - (constants::HAND_OFFSET_Y + constants::CARD_HEIGHT);
    else if (player == constants::P2)
        position.y = constants::HAND_OFFSET_Y;

    return IntRect(position, size);
}

vector<int> Game::getBoardIndices(int suit, int face)
{
    int cardID = suit * constants::NUM_FACES + face;
    vector<int> indices;

    for (int y = 0; y < constants::GAME_BOARD_SIZE; y++)
    {
        for (int x = 0; x < constants::GAME_BOARD_SIZE; x++)
        {
            int boardCard = constants::GAME_BOARD[y][x];
            if (boardCard == cardID)
            {
                indices.push_back(y * constants::GAME_BOARD_SIZE + x);
            }
        }
    }

    return indices;
}

void Game::update(RenderWindow& window)
{
    highlightedCard = -1;

    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            if (getHandRect(p, i).contains(Mouse::getPosition(window)))
            {
                Card card = hands[p][i];
                highlightedCard = card.suit * constants::NUM_FACES + card.face;
            }
        }
    }
}

void Game::draw(RenderWindow& window)
{
    window.draw(background);

    drawBoard(window);

    drawHands(window);
}

void Game::drawBoard(RenderWindow& window)
{
    Vector2f offset(constants::CARD_OFFSET_X, constants::CARD_OFFSET_Y);
    for (int y = 0; y < constants::GAME_BOARD_SIZE; y++)
    {
        for (int x = 0; x < constants::GAME_BOARD_SIZE; x++)
        {
            int cardID = constants::GAME_BOARD[y][x];
            int suit = cardID / constants::NUM_FACES;
            int face = cardID % constants::NUM_FACES;

            Sprite card;
            // If the card isn't wild,
            if (cardID != constants::WILD)
                // Set it to the proper sprite
                card = Sprite(cards[suit][face]);
            else
                // Otherwise, set it to the wildcard sprite (just a card back)
                card.setTexture(cardBack);
            card.setScale(constants::DRAWN_CARD_SCALE_FACTOR, constants::DRAWN_CARD_SCALE_FACTOR);

            Vector2f position((float)x * (constants::CARD_WIDTH + constants::CARD_GAP), (float)y * (constants::CARD_HEIGHT + constants::CARD_GAP));
            card.setPosition(position * constants::DRAWN_CARD_SCALE_FACTOR + offset);

            if (cardID == highlightedCard)
            {
                card.setColor(Color::Yellow);
            }

            window.draw(card);
        }
    }
}

void Game::drawHands(RenderWindow& window)
{
    Vector2f offset(constants::HAND_OFFSET_X, constants::SCREEN_HEIGHT - (constants::HAND_OFFSET_Y + constants::CARD_HEIGHT));
    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            Card card = hands[p][i];
            Sprite cardSprite = cards[card.suit][card.face];
            Vector2f position((float)i * (constants::CARD_WIDTH + constants::HAND_SPACING), 0);
            cardSprite.setPosition(position + offset);

            window.draw(cardSprite);
        }
        offset = Vector2f(constants::HAND_OFFSET_X, constants::HAND_OFFSET_Y);
    }
}