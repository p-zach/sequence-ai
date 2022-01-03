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
    loadTexture(tokenTextures[constants::P1], "token_blue.png");
    loadTexture(tokenTextures[constants::P2], "token_red.png");

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

    deck.clear();

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
            hands[p][i] = drawCard();
        }
    }
}

Card Game::drawCard()
{
    Card card = deck.back();
    deck.pop_back();
    return card;
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
    //printf("Turn: %d\n", (int)state);

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

    if (Mouse::isButtonPressed(Mouse::Button::Left))
    {
        for (int y = 0; y < constants::GAME_BOARD_SIZE; y++)
        {
            for (int x = 0; x < constants::GAME_BOARD_SIZE; x++)
            {
                if (getCardRect(x, y).contains(Mouse::getPosition(window)))
                {
                    clickCard(x, y);
                }
            }
        }
    }
}

void Game::clickCard(int x, int y)
{
    int index = y * 10 + x;

    // Exit if there is already a token on the clicked card
    if (tokenPositions[constants::P1].find(index) != end(tokenPositions[constants::P1]) ||
        tokenPositions[constants::P2].find(index) != end(tokenPositions[constants::P2]))
        return;

    int cardID = constants::GAME_BOARD[y][x];
    int suit = cardID / constants::NUM_FACES;
    int face = cardID % constants::NUM_FACES;

    // If the clicked card is in the current player's hand
    Card* cardInHand = find(begin(hands[(int)state]), end(hands[(int)state]), Card(suit, face));
    if (cardInHand != end(hands[(int)state]))
    {
        tokenPositions[(int)state].insert(index);

        *cardInHand = drawCard();

        state = (Game::GameState)(1 - (int)state);
    }
}

void Game::draw(RenderWindow& window)
{
    window.draw(background);

    drawBoard(window);

    drawHands(window);
}

Vector2f Game::getCardPosition(int x, int y)
{
    return Vector2f((float)x * (constants::CARD_WIDTH + constants::CARD_GAP), (float)y * (constants::CARD_HEIGHT + constants::CARD_GAP)) * constants::DRAWN_CARD_SCALE_FACTOR
        + Vector2f(constants::CARD_OFFSET_X, constants::CARD_OFFSET_Y);
}

IntRect Game::getCardRect(int x, int y)
{
    return IntRect((Vector2i)getCardPosition(x, y), Vector2i(Vector2f(constants::CARD_WIDTH, constants::CARD_HEIGHT) * constants::DRAWN_CARD_SCALE_FACTOR));
}

void Game::drawBoard(RenderWindow& window)
{
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
                // Otherwise, set it to the wildcard sprite (just the back of a card)
                card.setTexture(cardBack);
            card.setScale(constants::DRAWN_CARD_SCALE_FACTOR, constants::DRAWN_CARD_SCALE_FACTOR);

            card.setPosition(getCardPosition(x, y));

            if (cardID == highlightedCard)
            {
                card.setColor(Color::Yellow);
            }

            window.draw(card);
        }
    }

    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (auto position : tokenPositions[p])
        {
            drawToken(window, p, position);
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

void Game::drawToken(RenderWindow& window, int player, int index)
{
    int y = index / constants::GAME_BOARD_SIZE;
    int x = index % constants::GAME_BOARD_SIZE;

    Sprite token(tokenTextures[player]);
    token.setOrigin(constants::TOKEN_SIZE / 2, constants::TOKEN_SIZE / 2);
    token.setPosition(getCardPosition(x, y) + Vector2f(constants::CARD_WIDTH, constants::CARD_HEIGHT) * 0.5f * constants::DRAWN_CARD_SCALE_FACTOR);
    token.setScale(constants::TOKEN_SCALE_FACTOR, constants::TOKEN_SCALE_FACTOR);

    window.draw(token);
}