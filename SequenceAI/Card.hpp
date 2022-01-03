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
	IntRect getCardTextureBounds();

	static IntRect getCardTextureBounds(int suit, int face);
};