#pragma once

#include <SFML\Graphics.hpp>

using namespace sf;
using namespace std;

class Card
{
public:
	int suit;
	int face;

	Card();
	Card(int, int);
	IntRect getCardBoundsOnBoard();

	static IntRect getCardBoundsOnBoard(int suit, int face);
};