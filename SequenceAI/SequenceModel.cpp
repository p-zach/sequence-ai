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
    int index = y * 10 + x;

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

    function<int(Card*, bool)> handleClick = [this, usedCard, /*tokensP1, tokensP2,*/ index](Card* clicked, bool remove) {
        *usedCard = *clicked;

        if (remove)
        {
            // Remove clicked token
            //auto tokenIndex = state == GameState::TURN_P2 ? find(begin(tokensP1), end(tokensP1), index) : find(begin(tokensP2), end(tokensP2), index);
            auto tokenIndex = find(begin(tokenPositions[1 - (int)state]), end(tokenPositions[1 - (int)state]), index);
            tokenPositions[1 - (int)state].erase(tokenIndex);//begin(tokenPositions[1 - (int)state]) + index);
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
        /*startDiscardAnimation((int)state, distance(begin(hands[(int)state]), cardInHand), x, y, false);
        return;*/
        return handleClick(cardInHand, false);
    }

    // Check if hand contains a wildcard jack
    Card* wild = findCard(constants::SUIT_DIAMOND / 13, constants::FACE_JACK);
    if (wild == handEnd)
        wild = findCard(constants::SUIT_CLUB / 13, constants::FACE_JACK);

    if (wild != handEnd && !tokened)
    {
        /*startDiscardAnimation((int)state, distance(begin(hands[(int)state]), wild), x, y, false);
        return;*/
        return handleClick(wild, false);
    }

    // Check if hand contains a remove jack
    Card* remove = findCard(constants::SUIT_HEART / 13, constants::FACE_JACK);
    if (remove == handEnd)
        remove = findCard(constants::SUIT_SPADE / 13, constants::FACE_JACK);

    if (remove != handEnd && tokened && (tokenedP1 && state == GameState::TURN_P2 || tokenedP2 && state == GameState::TURN_P1))
    {
        //startDiscardAnimation((int)state, distance(begin(hands[(int)state]), remove), x, y, true);
        return handleClick(remove, true);
    }

    // If the clicked card is in the current player's hand
    //Card* cardInHand = find(begin(hands[(int)state]), end(hands[(int)state]), Card(suit, face));
    //if (cardInHand != end(hands[(int)state]))
    //{
    //    *usedCard = *cardInHand; 

    //    // Add token on top of clicked card
    //    tokenPositions[(int)state].push_back(index);

    //    // Draw new card and place into hand
    //    int index = distance(begin(hands[(int)state]), cardInHand);
    //    hands[(int)state][index] = drawCard();

    //    // Change state to other player's turn
    //    state = (GameState)(1 - (int)state);

    //    // Return card index in hand on successful placement
    //    return index;
    //}
    // Return invalid value on unsuccessful placement
    return constants::INVALID_CARD;
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