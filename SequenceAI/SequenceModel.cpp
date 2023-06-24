#include "SequenceModel.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string>

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
    Card card = deck.back();
    deck.pop_back();
    return card;
}

int SequenceModel::clickCard(int x, int y)
{
    int index = y * 10 + x;

    // Exit if there is already a token on the clicked card
    if (tokenPositions[constants::P1].find(index) != end(tokenPositions[constants::P1]) ||
        tokenPositions[constants::P2].find(index) != end(tokenPositions[constants::P2]))
        return constants::INVALID_CARD;

    int cardID = constants::GAME_BOARD[y][x];
    int suit = cardID / constants::NUM_FACES;
    int face = cardID % constants::NUM_FACES;

    // If the clicked card is in the current player's hand
    Card* cardInHand = find(begin(hands[(int)state]), end(hands[(int)state]), Card(suit, face));
    if (cardInHand != end(hands[(int)state]))
    {
        // Add token on top of clicked card
        tokenPositions[(int)state].insert(index);

        // Draw new card and place into hand
        *cardInHand = drawCard();

        // Change state to other player's turn
        state = (GameState)(1 - (int)state);

        // Return card ID on successful placement
        return cardID;
    }
    // Return invalid value on unsuccessful placement
    return constants::INVALID_CARD;
}

string SequenceModel::getTurnText() const
{
    return state == GameState::TURN_P1 ? "Your Turn" : "AI Turn";
}

Card SequenceModel::getHandCard(int player, int index) const
{
    if (player < 0 || player >= constants::NUM_PLAYERS || index < 0 || index > constants::HAND_SIZE)
        throw invalid_argument("getHandCard arguments must reference a valid card in a player's hand");
    return hands[player][index];
}

set<int> SequenceModel::getTokenPositions(int player) const
{
    if (player < 0 || player >= constants::NUM_PLAYERS)
        throw invalid_argument("getTokenPositions argument must be a valid player index");
    return tokenPositions[player];
}