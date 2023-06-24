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
	Card(int suit, int face);
	IntRect getCardTextureBounds();

	static IntRect getCardTextureBounds(int suit, int face);

	bool operator==(const Card&) const;

	static const Card invalid;
};