#include "Game.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <functional>

using namespace std;
using namespace sf;

Game::Game()
{
    loadContent();

    srand(time(0));
    reset();
}

/// <summary>
/// Loads a texture from disk.
/// </summary>
/// <param name="texture">The variable to save the texture to.</param>
/// <param name="file">The name of the texture file.</param>
void Game::loadTexture(Texture& texture, string file)
{
    // Load the texture
    if (!texture.loadFromFile(file))
    {
        // Print error if load error
        cout << "Error loading " << file << ". Aborting." << endl;
        exit(EXIT_FAILURE);
    }
    // Set texture smoothing to true
    texture.setSmooth(true);
}

/// <summary>
/// Loads the content for the game. Only needs to be called once.
/// </summary>
void Game::loadContent()
{
    // Load textures
    loadTexture(cardSheet, "cards.png");
    loadTexture(cardBack, "card_back.jpg");
    loadTexture(tokenTextures[constants::P1], "token_blue.png");
    loadTexture(tokenTextures[constants::P2], "token_red.png");
    // Load font
    if (!font.loadFromFile("NotoSans-Black.ttf"))
    {
        cout << "Error loading font file. Aborting." << endl;
        exit(EXIT_FAILURE);
    }

    // Set the background texture
    background = RectangleShape(Vector2f(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT));
    background.setFillColor(Color(0, constants::BACKGROUND_COLOR, 0));

    // Initialize card texture 2D array
    for (int suit = 0; suit < constants::NUM_SUITS; suit++)
    {
        for (int face = 0; face < constants::NUM_FACES; face++)
        {
            cards[suit][face].setTexture(cardSheet);
            cards[suit][face].setTextureRect(Card::getCardTextureBounds(suit, face));
        }
    }
}

/// <summary>
/// Resets the game to be played again.
/// </summary>
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

    // Draw cards from the deck for each player's hand
    for (int p = 0; p < constants::NUM_PLAYERS; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            hands[p][i] = drawCard();
        }
    }

    topDiscard = Sprite();
}

/// <summary>
/// Draws and removes a card from the top of the deck.
/// </summary>
/// <returns>The card at the top of the deck.</returns>
Card Game::drawCard()
{
    Card card = deck.back();
    deck.pop_back();
    return card;
}

/// <summary>
/// Gets the texture bounds for a card in a player's hand.
/// </summary>
/// <param name="player">The player holding the card.</param>
/// <param name="index">The index of the card in the player's hand.</param>
/// <returns>The texture bounds for the card.</returns>
IntRect Game::getHandRect(int player, int index)
{
    // Gets card size
    Vector2i size = Vector2i(constants::CARD_WIDTH, constants::CARD_HEIGHT);

    // Sets position
    Vector2i position;
    position.x = constants::HAND_OFFSET_X + (size.x + constants::HAND_SPACING) * index;
    if (player == constants::P1)
        position.y = constants::SCREEN_HEIGHT - (constants::HAND_OFFSET_Y + constants::CARD_HEIGHT);
    else if (player == constants::P2)
        position.y = constants::HAND_OFFSET_Y;

    return IntRect(position, size);
}

/// <summary>
/// Gets the board index of each card on the board matching the given suit and face.
/// </summary>
/// <param name="suit">The suit of the card to match.</param>
/// <param name="face">The face of the card to match.</param>
/// <returns>A vector of the board indices of the retrieved cards.</returns>
vector<int> Game::getBoardIndices(int suit, int face)
{
    // Calculates card ID
    int cardID = suit * constants::NUM_FACES + face;

    vector<int> indices;

    // Loop through board positions
    for (int y = 0; y < constants::GAME_BOARD_SIZE; y++)
    {
        for (int x = 0; x < constants::GAME_BOARD_SIZE; x++)
        {
            // Gets card ID at the board position
            int boardCard = constants::GAME_BOARD[y][x];
            // If the card at x, y is the desired card,
            if (boardCard == cardID)
            {
                // Add the board index to the list
                indices.push_back(y * constants::GAME_BOARD_SIZE + x);
            }
        }
    }

    return indices;
}

/// <summary>
/// Updates the game.
/// </summary>
/// <param name="window">The RenderWindow to draw to.</param>
/// <param name="elapsed">The seconds elapsed since the last update.</param>
void Game::update(RenderWindow& window, float elapsed)
{
    // If it's either player's turn,
    if (state == GameState::TURN_P1 || state == GameState::TURN_P2)
    {
        // Highlight selected cards on the board
        highlightSelectedCard(window);

        // Place a card if the player clicked on the board
        checkForCardClick(window);
    }
    // If an animation is playing,
    else if (state == GameState::ANIMATING)
    {
        // Update the animation
        updateAnimation(elapsed);

        //if (animated )/*
        //{

        //}*/
    }    
}

void Game::highlightSelectedCard(RenderWindow& window)
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

void Game::checkForCardClick(RenderWindow& window)
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

void Game::startAnimation(Sprite animated, Vector2f origin, Vector2f destination, GameState stateAfter, bool flip, function<void()> callAfter)
{
    this->animated = animated;
    animationOrigin = origin;
    animationDestination = destination;
    animationTime = 0.f;
    this->flip = flip;

    state = GameState::ANIMATING;
    stateAfterAnimation = stateAfter;
    functionToCall = callAfter;
}

void Game::updateAnimation(float elapsed)
{
    animationTime += elapsed;

    if (animationTime >= constants::ANIMATION_TIME)
    {
        state = stateAfterAnimation;
        functionToCall();
        return;
    }

    Vector2f diff = animationDestination - animationOrigin;
    Vector2f delta = diff * easeInOutCubic(animationTime / constants::ANIMATION_TIME);
    animationPosition = animationOrigin + delta;
}

float Game::easeInOutCubic(float x)
{
    return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
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
        startDiscardAnimation((int)state, distance(begin(hands[(int)state]), cardInHand), x, y);
    }
}

void Game::startDiscardAnimation(int player, int handIndex, int x, int y)
{
    IntRect handRect = getHandRect(player, handIndex);

    Card* card = &hands[player][handIndex];
    Sprite* cardSprite = &cards[card->suit][card->face];
    hands[player][handIndex] = Card::invalid;

    function<void()> afterDiscard = [this, cardSprite, player, x, y, handIndex]() { 
        topDiscard = Sprite(*cardSprite); 
        startTokenPlaceAnimation(player, x, y, handIndex);
    };

    startAnimation(*cardSprite, Vector2f(handRect.left, handRect.top),
        (Vector2f)getDiscardPosition(), (Game::GameState)(1 - (int)state), false,
        afterDiscard);
}

void Game::startTokenPlaceAnimation(int player, int x, int y, int handIndex)
{
    int index = y * 10 + x;

    Sprite tokenSprite(tokenTextures[player]);
    tokenSprite.setOrigin(constants::TOKEN_SIZE / 2, constants::TOKEN_SIZE / 2);
    tokenSprite.setScale(constants::TOKEN_SCALE_FACTOR, constants::TOKEN_SCALE_FACTOR);

    function<void()> afterTokenPlace = [this, index, player, handIndex]() {
        tokenPositions[player].insert(index);
        startDrawCardAnimation(player, handIndex);
    };

    startAnimation(tokenSprite, 
        Vector2f(constants::TOKEN_ANIM_START_X, player ? constants::TOKEN_ANIM_START_Y_P2 : constants::TOKEN_ANIM_START_Y_P1), 
        getCardPosition(x, y) + Vector2f(constants::CARD_WIDTH, constants::CARD_HEIGHT) * 0.5f * constants::DRAWN_CARD_SCALE_FACTOR, 
        state, false, afterTokenPlace);
}

void Game::startDrawCardAnimation(int player, int index)
{
    IntRect handRect = getHandRect(player, index);
    Card newCard = drawCard();

    function<void()> afterDrawCard = [this, player, index, newCard]() {
        hands[player][index] = newCard;
    };

    startAnimation(cards[newCard.suit][newCard.face], (Vector2f)getDrawPosition(), Vector2f(handRect.left, handRect.top), state, true, afterDrawCard);
}

void Game::draw(RenderWindow& window)
{
    window.draw(background);

    drawBoard(window);

    drawHands(window);

    drawInformation(window);

    if (state == GameState::ANIMATING)
    {
        drawAnimation(window);
    }
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

    Sprite topDraw(cardBack);

    topDraw.setPosition((Vector2f)getDrawPosition());

    window.draw(topDraw);

    topDiscard.setPosition((Vector2f)getDiscardPosition());

    window.draw(topDiscard);
}

void Game::drawHands(RenderWindow& window)
{
    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            if (hands[p][i] == Card::invalid)
                continue;
            Card card = hands[p][i];
            Sprite cardSprite = cards[card.suit][card.face];
            IntRect handRect = getHandRect(p, i);
            Vector2f position = Vector2f(handRect.left, handRect.top);
            cardSprite.setPosition(position);

            window.draw(cardSprite);
        }
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

void Game::drawInformation(RenderWindow& window)
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

    Text turn;
    turn.setFont(font);
    turn.setString(state == GameState::TURN_P1 ? "Your Turn" : state == GameState::TURN_P2 ? "AI Turn" : "");
    turn.setFillColor(Color::Black);
    turn.setCharacterSize(constants::TURN_TEXT_SIZE);
    turn.setPosition(constants::TURN_OFFSET_X + (state == GameState::TURN_P2 ? constants::TURN_EXTRA_OFFSET_X : 0), constants::TURN_OFFSET_Y);

    window.draw(turn);
}

void Game::drawAnimation(RenderWindow& window)
{
    if (flip)
    {
        Sprite toDraw;
        float t = easeInOutCubic(animationTime);

        if (animationTime < constants::ANIMATION_TIME / 2)
        {
            toDraw = Sprite(cardBack);
            toDraw.setScale(1 - (2 / constants::ANIMATION_TIME * animationTime), 1);
        }
        else
        {
            toDraw = Sprite(animated);
            toDraw.setScale((animationTime - constants::ANIMATION_TIME / 2) / (constants::ANIMATION_TIME / 2), 1);
        }
        //toDraw.setOrigin(1 - abs(2 * t / constants::ANIMATION_TIME - 1), 0);
        //toDraw.setOrigin(toDraw.getTexture()->getSize().x /** (1 / toDraw.getScale().x)*/ * (.5f - abs(animationTime / constants::ANIMATION_TIME - .5f)), 0);
        toDraw.setPosition(animationPosition);
        window.draw(toDraw);
    }
    else
    {
        animated.setPosition(animationPosition);
        window.draw(animated);
    }
}

Vector2i Game::getDrawPosition()
{
    return Vector2i(constants::STACK_OFFSET_X, constants::STACK_OFFSET_Y);
}

Vector2i Game::getDiscardPosition()
{
    return Vector2i(constants::STACK_OFFSET_X + constants::CARD_WIDTH + constants::STACK_SPACING, constants::STACK_OFFSET_Y);
}