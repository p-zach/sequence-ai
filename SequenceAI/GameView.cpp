#include "GameView.hpp"
#include "GameController.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace sf;

GameView::GameView(GameController& controller) : controller(controller)
{
	loadContent();
}

void GameView::reset()
{
    topDiscard = Sprite();
}

/// <summary>
/// Loads an SFML texture from a file.
/// </summary>
/// <param name="texture">The texture to load to.</param>
/// <param name="file">The texture file name.</param>
void GameView::loadTexture(Texture& texture, string file)
{
    if (!texture.loadFromFile(file))
    {
        cout << "Error loading " << file << ". Aborting." << endl;
        exit(EXIT_FAILURE);
    }
    texture.setSmooth(true);
}

/// <summary>
/// Loads visual content for the game.
/// </summary>
void GameView::loadContent()
{
    loadTexture(cardSheet, "cards.png");
    loadTexture(cardBack, "card_back.jpg");
    loadTexture(tokenTextures[constants::P1], "token_blue.png");
    loadTexture(tokenTextures[constants::P2], "token_red.png");
    if (!font.loadFromFile("NotoSans-Black.ttf"))
    {
        cout << "Error loading font file. Aborting." << endl;
        exit(EXIT_FAILURE);
    }

    background = RectangleShape(Vector2f(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT));
    background.setFillColor(Color(0, constants::BACKGROUND_COLOR, 0));

    for (int suit = 0; suit < constants::NUM_SUITS; suit++)
    {
        for (int face = 0; face < constants::NUM_FACES; face++)
        {
            cards[suit][face].setTexture(cardSheet);
            cards[suit][face].setTextureRect(Card::getCardTextureBounds(suit, face));
        }
    }
}

void GameView::update(RenderWindow& window, float elapsed)
{
    highlightSelectedCard(window);

    checkForCardClick(window);

    // belongs in something like updateAnimation called by Controller instead of this function when animating
    //else if (state == GameState::ANIMATING)
    //{
    //    updateAnimation(elapsed);

    //    //if (animated )/*
    //    //{

    //    //}*/
    //}
}

void GameView::clickCard(int x, int y)
{
    // Try to place a token
    int clicked = controller.clickCard(x, y);
    if (clicked != constants::INVALID_CARD)
    {
        // If the token was placed, update the discard pile with the used card
        topDiscard = Sprite(cards[clicked / constants::NUM_FACES][clicked % constants::NUM_FACES]);
    }
}

IntRect GameView::getHandRect(int player, int index)
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

vector<int> GameView::getBoardIndices(int suit, int face)
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

void GameView::highlightSelectedCard(RenderWindow& window)
{
    highlightedCard = -1;

    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            if (getHandRect(p, i).contains(Mouse::getPosition(window)))
            {
                Card card = controller.getHandCard(p, i);
                highlightedCard = card.suit * constants::NUM_FACES + card.face;
            }
        }
    }
}

void GameView::checkForCardClick(RenderWindow& window)
{
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

//void GameView::startAnimation(Sprite animated, Vector2f origin, Vector2f destination, GameState stateAfter, bool flip)
//{
//    this->animated = animated;
//    animationOrigin = origin;
//    animationDestination = destination;
//    this->flip = flip;
//
//    state = GameState::ANIMATING;
//    stateAfterAnimation = stateAfter;
//}
//
//void GameView::updateAnimation(float elapsed)
//{
//    animationTime += elapsed;
//
//    if (animationTime >= constants::ANIMATION_TIME)
//    {
//        state = stateAfterAnimation;
//        return;
//    }
//
//    Vector2f diff = animationDestination - animationOrigin;
//    Vector2f delta = diff * easeInOutCubic(animationTime / constants::ANIMATION_TIME);
//    animationPosition = animationOrigin + delta;
//}
//
//float GameView::easeInOutCubic(float x)
//{
//    return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
//}

void GameView::draw(RenderWindow& window)
{
    window.draw(background);

    drawBoard(window);

    drawHands(window);

    drawInformation(window);
}

Vector2f GameView::getCardPosition(int x, int y)
{
    return Vector2f((float)x * (constants::CARD_WIDTH + constants::CARD_GAP), (float)y * (constants::CARD_HEIGHT + constants::CARD_GAP)) * constants::DRAWN_CARD_SCALE_FACTOR
        + Vector2f(constants::CARD_OFFSET_X, constants::CARD_OFFSET_Y);
}

IntRect GameView::getCardRect(int x, int y)
{
    return IntRect((Vector2i)getCardPosition(x, y), Vector2i(Vector2f(constants::CARD_WIDTH, constants::CARD_HEIGHT) * constants::DRAWN_CARD_SCALE_FACTOR));
}

void GameView::drawBoard(RenderWindow& window)
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
        for (auto position : controller.getTokenPositions(p))
        {
            drawToken(window, p, position);
        }
    }

    Sprite topDraw(cardBack);

    Vector2f position(constants::STACK_OFFSET_X, constants::STACK_OFFSET_Y);

    topDraw.setPosition(position);

    window.draw(topDraw);

    topDiscard.setPosition(position + Vector2f(constants::CARD_WIDTH + constants::STACK_SPACING, 0));

    window.draw(topDiscard);
}

void GameView::drawHands(RenderWindow& window)
{
    Vector2f offset(constants::HAND_OFFSET_X, constants::SCREEN_HEIGHT - (constants::HAND_OFFSET_Y + constants::CARD_HEIGHT));
    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            Card card = controller.getHandCard(p, i);
            if (card == Card::invalid)
                continue;
            Sprite cardSprite = cards[card.suit][card.face];
            Vector2f position((float)i * (constants::CARD_WIDTH + constants::HAND_SPACING), 0);
            cardSprite.setPosition(position + offset);

            window.draw(cardSprite);
        }
        offset = Vector2f(constants::HAND_OFFSET_X, constants::HAND_OFFSET_Y);
    }
}

void GameView::drawToken(RenderWindow& window, int player, int index)
{
    int y = index / constants::GAME_BOARD_SIZE;
    int x = index % constants::GAME_BOARD_SIZE;

    Sprite token(tokenTextures[player]);
    token.setOrigin(constants::TOKEN_SIZE / 2, constants::TOKEN_SIZE / 2);
    token.setPosition(getCardPosition(x, y) + Vector2f(constants::CARD_WIDTH, constants::CARD_HEIGHT) * 0.5f * constants::DRAWN_CARD_SCALE_FACTOR);
    token.setScale(constants::TOKEN_SCALE_FACTOR, constants::TOKEN_SCALE_FACTOR);

    window.draw(token);
}

void GameView::drawInformation(RenderWindow& window)
{
    Text tip1, tip2;
    tip1.setFont(font);
    tip2.setFont(font);

    tip1.setString("ONE EYED JACKS REMOVE");
    tip2.setString("TWO EYED JACKS ARE WILD");

    tip1.setFillColor(Color(0, constants::TIP_COLOR, 0));
    tip2.setFillColor(Color(0, constants::TIP_COLOR, 0));

    tip1.setCharacterSize(constants::TIP_TEXT_SIZE);
    tip2.setCharacterSize(constants::TIP_TEXT_SIZE);

    tip1.setPosition(constants::TIP_1_OFFSET_X, constants::TIP_OFFSET_Y);
    tip2.setPosition(constants::TIP_2_OFFSET_X, constants::SCREEN_HEIGHT - (constants::TIP_OFFSET_Y + constants::TIP_TEXT_SIZE));

    window.draw(tip1);
    window.draw(tip2);

    Text turn{ controller.getTurnText(), font };
    turn.setFillColor(Color::Black);
    turn.setCharacterSize(constants::TURN_TEXT_SIZE);
    turn.setPosition(constants::TURN_OFFSET_X - turn.getGlobalBounds().width / 2.f, constants::TURN_OFFSET_Y);

    window.draw(turn);
}