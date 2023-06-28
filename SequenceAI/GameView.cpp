#include "GameView.hpp"
#include "GameController.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <functional>

using namespace std;
using namespace sf;

GameView::GameView(GameController& controller) : controller(controller)
{
	loadContent();
    reset();
    usedCard = new Card();
}

GameView::~GameView()
{
    delete(usedCard);
}

void GameView::reset()
{
    topDiscard = Sprite();
    tempTokenPos = -1;
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

    drawNewlyDrawnCard = true;
    drawLastToken = true;
}

void GameView::update(RenderWindow& window, float elapsed)
{
    highlightSelectedCard(window);

    checkForCardClick(window);
}

void GameView::clickCard(int x, int y)
{
    // Try to place a token
    int clicked = controller.clickCard(x, y, usedCard);
    if (clicked != constants::INVALID_CARD)
    {
        startDiscardAnimation(1 - controller.getPlayerIndex(), clicked, x, y);
    }
    controller.checkWin(1 - controller.getPlayerIndex(), x, y);
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
    highlightedCard = constants::HIGHLIGHT_NONE;

    // Don't highlight if the game is over
    if (controller.gameIsWon() != -1)
        return;

    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            if (getHandRect(p, i).contains(Mouse::getPosition(window)))
            {
                Card card = controller.getHandCard(p, i);

                // Regular cards just highlight themselves on the board
                if (card.face != constants::FACE_JACK)
                {
                    highlightedCard = card.suit * constants::NUM_FACES + card.face;
                }
                // Jacks have special capabilities
                else
                {
                    // One-eyed jacks can remove tokens
                    if (card.suit == constants::SUIT_HEART / 13 || card.suit == constants::SUIT_SPADE / 13)
                    {
                        highlightedCard = constants::HIGHLIGHT_TOKENED_P1 - p;
                    }
                    // Two-eyed jacks are wildcards
                    else if (card.suit == constants::SUIT_DIAMOND / 13 || card.suit == constants::SUIT_CLUB / 13)
                    {
                        highlightedCard = constants::HIGHLIGHT_ALL;
                    }
                }
            }
        }
    }
}

void GameView::checkForCardClick(RenderWindow& window)
{
    // Don't highlight if the game is over
    if (controller.gameIsWon() != -1)
        return;

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

void GameView::startAnimation(Sprite animated, Vector2f origin, Vector2f destination, bool flip, function<void()> callAfter)
{
    currentlyAnimating = true;
    this->animated = animated;
    animationOrigin = origin;
    animationDestination = destination;
    animationTime = 0.f;
    this->flip = flip;

    functionAfterAnimation = callAfter;
}

void GameView::updateAnimation(float elapsed)
{
    animationTime += elapsed;

    if (animationTime >= constants::ANIMATION_TIME)
    {
        // prevent 1-frame flashing of following animated sprite
        animationPosition = Vector2f(-1000, -1000);

        currentlyAnimating = false;
        functionAfterAnimation();
        return;
    }

    Vector2f diff = animationDestination - animationOrigin;
    Vector2f delta = diff * easeInOutCubic(animationTime / constants::ANIMATION_TIME);
    animationPosition = animationOrigin + delta;
}

float GameView::easeInOutCubic(float x)
{
    return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
}

void GameView::drawAnimation(RenderWindow& window)
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
        toDraw.setPosition(animationPosition);
        window.draw(toDraw);
    }
    else
    {
        animated.setPosition(animationPosition);
        window.draw(animated);
    }

}

Vector2i GameView::getDrawPosition()
{
    return Vector2i(constants::STACK_OFFSET_X, constants::STACK_OFFSET_Y);
}

Vector2i GameView::getDiscardPosition()
{
    return Vector2i(constants::STACK_OFFSET_X + constants::CARD_WIDTH + constants::STACK_SPACING, constants::STACK_OFFSET_Y);
}

void GameView::startDiscardAnimation(int player, int handIndex, int x, int y)
{
    IntRect handRect = getHandRect(player, handIndex);

    Card card = *usedCard;
    Sprite cardSprite = Sprite(cards[card.suit][card.face]);

    bool removingToken = false;
    if (card.face == constants::FACE_JACK && (card.suit == constants::SUIT_HEART / 13 || card.suit == constants::SUIT_SPADE / 13))
        removingToken = true;

    function<void()> afterDiscard = [this, cardSprite, player, x, y, handIndex, removingToken]() {
        topDiscard = Sprite(cardSprite);
        tempTokenPos = -1;
        startTokenPlaceAnimation(player, x, y, handIndex, removingToken);
    };

    startAnimation(cardSprite, Vector2f(handRect.left, handRect.top),
        (Vector2f)getDiscardPosition(), false,
        afterDiscard);

    discardPlayer = player;
    discardIndex = handIndex;
    
    drawLastToken = removingToken;
    drawNewlyDrawnCard = false;

    if (removingToken)
        tempTokenPos = y * constants::GAME_BOARD_SIZE + x;
}

void GameView::startTokenPlaceAnimation(int player, int x, int y, int handIndex, bool remove)
{
    int index = y * constants::GAME_BOARD_SIZE + x;

    Sprite tokenSprite(tokenTextures[remove ? 1 - player : player]);
    tokenSprite.setOrigin(constants::TOKEN_SIZE / 2, constants::TOKEN_SIZE / 2);
    tokenSprite.setScale(constants::TOKEN_SCALE_FACTOR, constants::TOKEN_SCALE_FACTOR);

    Vector2f origin(constants::TOKEN_ANIM_START_X, player ? constants::TOKEN_ANIM_START_Y_P2 : constants::TOKEN_ANIM_START_Y_P1);
    Vector2f dest = getCardPosition(x, y) + Vector2f(constants::CARD_WIDTH, constants::CARD_HEIGHT) * 0.5f * constants::DRAWN_CARD_SCALE_FACTOR;
    function<void()> afterTokenPlace;

    if (!remove)
    {
        afterTokenPlace = [this, index, player, handIndex]() {
            //tokenPositions[player].insert(index);
            startDrawCardAnimation(player, handIndex);
            drawLastToken = true;
        };
    }
    else
    {
        origin = dest;
        dest = Vector2f(constants::TOKEN_ANIM_START_X, player ? constants::TOKEN_ANIM_START_Y_P1 : constants::TOKEN_ANIM_START_Y_P2);
        //tokenPositions[1 - player].erase(index);
        afterTokenPlace = [this, player, handIndex]() {
            startDrawCardAnimation(player, handIndex);
            drawLastToken = true;
        };
    }

    startAnimation(tokenSprite, origin, dest, false, afterTokenPlace);
}

void GameView::startDrawCardAnimation(int player, int index)
{
    IntRect handRect = getHandRect(player, index);
    Card newCard = controller.getHandCard(discardPlayer, discardIndex);

    function<void()> afterDrawCard = [this, player, index, newCard]() {
        drawNewlyDrawnCard = true;
    };

    startAnimation(cards[newCard.suit][newCard.face], (Vector2f)getDrawPosition(), Vector2f(handRect.left, handRect.top), true, afterDrawCard);
}

void GameView::draw(RenderWindow& window)
{
    window.draw(background);

    drawBoard(window);

    drawHands(window);

    drawInformation(window);

    drawAnimation(window);
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
                // Otherwise, set it to the wildcard sprite (the back of a card)
                card.setTexture(cardBack);
            card.setScale(constants::DRAWN_CARD_SCALE_FACTOR, constants::DRAWN_CARD_SCALE_FACTOR);

            card.setPosition(getCardPosition(x, y));

            // Check whether this card has a token on it
            auto tokensP1 = controller.getTokenPositions(constants::P1);
            auto tokensP2 = controller.getTokenPositions(constants::P2);
            bool tokenedP1 = find(begin(tokensP1), end(tokensP1), y * constants::GAME_BOARD_SIZE + x) != end(tokensP1);
            bool tokenedP2 = find(begin(tokensP2), end(tokensP2), y * constants::GAME_BOARD_SIZE + x) != end(tokensP2);
            bool tokened = tokenedP1 || tokenedP2;

            // If the card specifically is selected, a wildcard jack is selected, or it's tokened and a remove jack is selected,
            if (cardID != constants::WILD && (
                (cardID == highlightedCard && !tokened)
                || (!tokened && highlightedCard == constants::HIGHLIGHT_ALL)
                || (tokenedP1 && highlightedCard == constants::HIGHLIGHT_TOKENED_P2 && !controller.inFirstSequence(constants::P1, x, y))
                || (tokenedP2 && highlightedCard == constants::HIGHLIGHT_TOKENED_P1 && !controller.inFirstSequence(constants::P2, x, y)))
                )
            {
                // Highlight the card
                card.setColor(Color::Yellow);
            }

            window.draw(card);
        }
    }

    for (int p = constants::P1; p <= constants::P2; p++)
    {
        auto tokens = controller.getTokenPositions(p);
        for (int t = 0; t < (drawLastToken || p != discardPlayer ? tokens.size() : tokens.size() - 1); t++)
        {
            drawToken(window, p, tokens[t]);
        }
    }
    if (tempTokenPos != -1)
        drawToken(window, controller.getPlayerIndex(), tempTokenPos);

    // Only draw discard and draw if game is still going on
    if (currentlyAnimating || controller.gameIsWon() == -1)
    {
        Sprite topDraw(cardBack);

        topDraw.setPosition((Vector2f)getDrawPosition());

        window.draw(topDraw);

        topDiscard.setPosition((Vector2f)getDiscardPosition());

        window.draw(topDiscard);
    }
}

void GameView::drawHands(RenderWindow& window)
{
    Vector2f offset(constants::HAND_OFFSET_X, constants::SCREEN_HEIGHT - (constants::HAND_OFFSET_Y + constants::CARD_HEIGHT));
    for (int p = constants::P1; p <= constants::P2; p++)
    {
        for (int i = 0; i < constants::HAND_SIZE; i++)
        {
            if (!drawNewlyDrawnCard && p == discardPlayer && i == discardIndex)
                continue;
            Card card = controller.getHandCard(p, i);
            if (card == Card::invalid)
                continue;
            Sprite cardSprite = cards[card.suit][card.face];
            IntRect handRect = getHandRect(p, i);
            Vector2f position = Vector2f(handRect.left, handRect.top);
            cardSprite.setPosition(position);

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

    if (!currentlyAnimating && controller.gameIsWon() != -1)
    {
        Text won{ controller.gameIsWon() == 0 ? "YOU WIN!" : "AI WINS!", font };
        won.setFillColor(Color::Black);
        won.setCharacterSize(constants::WIN_TEXT_SIZE);
        won.setPosition(constants::WIN_OFFSET_X - won.getGlobalBounds().width / 2.f, constants::WIN_OFFSET_Y);

        window.draw(won);
    }
    else if (!currentlyAnimating)
    {
        Text turn{ controller.getPlayerIndex() == 0 ? "Your Turn" : "AI Turn", font };
        turn.setFillColor(Color::Black);
        turn.setCharacterSize(constants::TURN_TEXT_SIZE);
        turn.setPosition(constants::TURN_OFFSET_X - turn.getGlobalBounds().width / 2.f, constants::TURN_OFFSET_Y);

        window.draw(turn);
    }
}

bool GameView::isAnimating() const
{
    return currentlyAnimating;
}

bool GameView::needDoubleUpdate() const
{
    return animationTime <= 0.f;
}