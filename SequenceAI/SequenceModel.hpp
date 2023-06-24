#pragma once

#include "Card.hpp"
#include "Constants.hpp"

#include <set>
#include <vector>
#include <string>

using namespace sf;
using namespace std;

class SequenceModel
{
public:
	enum class GameState { TURN_P1, TURN_P2 };

	SequenceModel();

	int clickCard(int x, int y);

	string getTurnText() const;
	Card getHandCard(int player, int index) const;
	set<int> getTokenPositions(int player) const;

private:
	GameState state;

	vector<Card> deck;
	Card hands[constants::NUM_PLAYERS][constants::HAND_SIZE];

	set<int> tokenPositions[constants::NUM_PLAYERS];

	void reset();
	Card drawCard();

};

