#include "SequenceModel.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string>
#include <functional>

SequenceModel::SequenceModel()
{
    srand(time(0));
    reset();
}

void SequenceModel::reset()
{
    state = GameState::TURN_P1;
    winner = -1;

    deck.clear();

    // Initialize deck
    for (int i = 0; i < constants::DECK_SIZE; i++)
    {
        // cheat mode (all wilds)
        //deck.push_back(Card(3, 10));
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

    for (int p = 0; p < constants::NUM_PLAYERS; p++)
    {
        for (int i = 0; i < constants::SEQUENCE_LENGTH; i++)
        {
            firstSequence[p][i] = -1;
        }
    }
}

Card SequenceModel::drawCard()
{
    if (deck.size() == 0)
        return Card::invalid;
    Card card = Card(deck.back());
    deck.pop_back();
    return card;
}

int SequenceModel::clickCard(int x, int y, Card* usedCard)
{
    int index = y * constants::GAME_BOARD_SIZE + x;

    // Can't place a token on wildcards
    if (index == 0
        || index == constants::GAME_BOARD_SIZE - 1
        || index == constants::GAME_BOARD_SIZE * constants::GAME_BOARD_SIZE - constants::GAME_BOARD_SIZE
        || index == constants::GAME_BOARD_SIZE * constants::GAME_BOARD_SIZE - 1)
        return constants::INVALID_CARD;

    // Check whether this card has a token on it
    auto tokensP1 = getTokenPositions(constants::P1);
    auto tokensP2 = getTokenPositions(constants::P2);
    bool tokenedP1 = find(begin(tokensP1), end(tokensP1), index) != end(tokensP1);
    bool tokenedP2 = find(begin(tokensP2), end(tokensP2), index) != end(tokensP2);
    bool tokened = tokenedP1 || tokenedP2;

    int cardID = constants::GAME_BOARD[y][x];
    int suit = cardID / constants::NUM_FACES;
    int face = cardID % constants::NUM_FACES;

    function<Card * (int, int)> findCard = [this](int s, int f) {
        return find(begin(hands[(int)state]), end(hands[(int)state]), Card(s, f));
    };
    Card* handEnd = end(hands[(int)state]);

    function<int(Card*, bool)> handleClick = [this, usedCard, index](Card* clicked, bool remove) {
        *usedCard = *clicked;

        if (remove)
        {
            // Remove clicked token
            auto tokenIndex = find(begin(tokenPositions[1 - (int)state]), end(tokenPositions[1 - (int)state]), index);
            tokenPositions[1 - (int)state].erase(tokenIndex);
        }
        else
        {
            // Add token on top of clicked card
            tokenPositions[(int)state].push_back(index);
        }

        // Draw new card and place into hand
        int newIndex = distance(begin(hands[(int)state]), clicked);
        hands[(int)state][newIndex] = drawCard();

        // Change state to other player's turn
        state = (GameState)(1 - (int)state);
        
        // Return card index in hand on successful placement
        return newIndex;
    };

    // If the clicked card is in the current player's hand and there's no token on it
    Card* cardInHand = findCard(suit, face);
    if (cardInHand != handEnd && !tokened)
    {
        return handleClick(cardInHand, false);
    }

    // Check if hand contains a wildcard jack
    Card* wild = findCard(constants::SUIT_DIAMOND / 13, constants::FACE_JACK);
    if (wild == handEnd)
        wild = findCard(constants::SUIT_CLUB / 13, constants::FACE_JACK);

    if (wild != handEnd && !tokened)
    {
        return handleClick(wild, false);
    }

    // Check if hand contains a remove jack
    Card* remove = findCard(constants::SUIT_HEART / 13, constants::FACE_JACK);
    if (remove == handEnd)
        remove = findCard(constants::SUIT_SPADE / 13, constants::FACE_JACK);

    if (remove != handEnd && tokened && 
        (tokenedP1 && state == GameState::TURN_P2 && !inFirstSequence(constants::P1, x, y) 
            || tokenedP2 && state == GameState::TURN_P1 && !inFirstSequence(constants::P2, x, y)))
    {
        return handleClick(remove, true);
    }

    // Return invalid value on unsuccessful placement
    return constants::INVALID_CARD;
}

bool SequenceModel::checkWin(int player, int placedX, int placedY)
{    
    // Helper function to check if a board position contains a token of the correct color or a wildcard
    function<bool(int, int)> tokenExists = [this, player](int x, int y) {
        if (x < 0 || x >= constants::GAME_BOARD_SIZE || y < 0 || y >= constants::GAME_BOARD_SIZE)
            return false;
        int token = y * constants::GAME_BOARD_SIZE + x;
        return (bool)count(tokenPositions[player].begin(), tokenPositions[player].end(), token) || constants::GAME_BOARD[y][x] == constants::WILD;
    };

    int minX = placedX - constants::SEQUENCE_LENGTH + 1;
    int maxX = placedX + constants::SEQUENCE_LENGTH;
    int minY = placedY - constants::SEQUENCE_LENGTH + 1;
    int maxY = placedY + constants::SEQUENCE_LENGTH;

    // Player has first sequence already if their firstSequence has been initialized
    bool hasFirstSequence = firstSequence[player][0] != -1;

    // Check if newly placed token causes a sequence to be created

    auto checkForSequence = [this, hasFirstSequence, tokenExists, player](int minX, int minY, int maxX, int maxY, int dirX, int dirY) {
        bool takenOneFromFirstSequence = false;
        int sequenceCount = 0;
        int x = minX;
        int y = minY;
        while (x < maxX && y < maxY)
        {
            // If there's a token on this card
            if (tokenExists(x, y))
            {
                // If the token is from an existing sequence
                int token = y * constants::GAME_BOARD_SIZE + x;
                if (count(firstSequence[player], firstSequence[player] + constants::SEQUENCE_LENGTH, token))
                {
                    // If we've already taken a token from the existing sequence
                    if (takenOneFromFirstSequence)
                    {
                        // Can't borrow more than 1 token from the first sequence; need to reset count to use just this token
                        sequenceCount = 1;
                    }
                    // If we haven't already taken a token from the existing sequence
                    else
                    {
                        // Use the token from the existing sequence
                        sequenceCount++;
                        // Remember that we've used a token from the existing sequence
                        takenOneFromFirstSequence = true;
                    }
                }
                // If the token isn't from an existing sequence, increment token count
                else sequenceCount++;
                // If the new token creates a second sequence, return true
                if (hasFirstSequence && sequenceCount == constants::SEQUENCE_LENGTH)
                    return true;
                // If the new token creates the first sequence, make the array contain the tokens used in the sequence we just discovered
                else if (sequenceCount == constants::SEQUENCE_LENGTH)
                {
                    // Walk backwards in the direction (-dirX, -dirY) to find the SEQUENCE_LENGTH (probably 5) tokens used
                    for (int i = 0; i > -constants::SEQUENCE_LENGTH; i--)
                    {
                        firstSequence[player][constants::SEQUENCE_LENGTH + i - 1] = (y * constants::GAME_BOARD_SIZE + x) 
                            + i * (dirY * constants::GAME_BOARD_SIZE + dirX);
                    }
                    // If we're constructing the first sequence, we definitely don't have 2 sequences. Therefore this token did not win
                    return false;
                }
            }
            // If there's no token on the card, reset sequence count to 0
            else sequenceCount = 0;

            x += dirX;
            y += dirY;
        }
        return false;
    };
    bool temp = checkForSequence(minX, placedY, maxX, placedY + 1, 1, 0) || checkForSequence(placedX, minY, placedX + 1, maxY, 0, 1) ||
        checkForSequence(minX, minY, maxX, maxY, 1, 1) || checkForSequence(minX, maxY - 1, maxX, maxY, 1, -1);
    if (temp) {
        winner = player;
    }
    return temp;
}

int SequenceModel::gameIsWon() const
{
    return winner;
}

int SequenceModel::getPlayerIndex() const
{
    return (int)state;
}

Card SequenceModel::getHandCard(int player, int index) const
{
    if (player < 0 || player >= constants::NUM_PLAYERS || index < 0 || index > constants::HAND_SIZE)
        throw invalid_argument("getHandCard arguments must reference a valid card in a player's hand");
    return hands[player][index];
}

vector<int> SequenceModel::getTokenPositions(int player) const
{
    if (player < 0 || player >= constants::NUM_PLAYERS)
        throw invalid_argument("getTokenPositions argument must be a valid player index");
    return tokenPositions[player];
}

bool SequenceModel::inFirstSequence(int player, int x, int y) const
{
    if (player < 0 || player >= constants::NUM_PLAYERS)
        throw invalid_argument("inFirstSequence player argument must be a valid player index");
    if (x < 0 || x >= constants::GAME_BOARD_SIZE || y < 0 || y >= constants::GAME_BOARD_SIZE)
        throw invalid_argument("inFirstSequence x and y indices must be in range");
    return (bool)count(firstSequence[player], firstSequence[player] + constants::SEQUENCE_LENGTH, y * constants::GAME_BOARD_SIZE + x);
}