#include "Card.hpp"
#include "constants.hpp"

Card::Card() : Card(0, 0) { }

Card::Card(int suit, int face) : suit(suit), face(face)
{
}

IntRect Card::getCardTextureBounds()
{
	return getCardTextureBounds(suit, face);
}

// Gets the card's bounds on the texture sheet.
IntRect Card::getCardTextureBounds(int suit, int face)
{
	return IntRect(face * constants::CARD_WIDTH, suit * constants::CARD_HEIGHT, constants::CARD_WIDTH, constants::CARD_HEIGHT);
}