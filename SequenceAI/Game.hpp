#pragma once

#include <SFML\Graphics.hpp>
#include "Card.hpp"
#include "Constants.hpp"

using namespace sf;
using namespace std;

class Game
{
public:
	enum GameState { TURN_P1, TURN_P2, WON };

	Game();
	void draw(RenderWindow&);

private:
	GameState state;

	Card deck[constants::DECK_SIZE];
	Card hands[2][constants::HAND_SIZE];
};