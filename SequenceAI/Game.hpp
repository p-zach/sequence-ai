#pragma once

#include <SFML\Graphics.hpp>
#include "Card.hpp"
#include "Constants.hpp"

#include <vector>

using namespace sf;
using namespace std;

class Game
{
public:
	enum class GameState { TURN_P1, TURN_P2, WON };

	Game();
	void update(RenderWindow&);
	void draw(RenderWindow&);

private:
	GameState state;

	vector<Card> deck;
	Card hands[constants::NUM_PLAYERS][constants::HAND_SIZE];

	Texture cardSheet;
	Texture cardBack;
	Texture tokens[constants::NUM_PLAYERS];
	Sprite cards[constants::NUM_SUITS][constants::NUM_FACES];
	RectangleShape background;

	int highlightedCard;

	void loadTexture(Texture&, string);
	void loadContent();
	void reset();

	IntRect getHandRect(int player, int index);
	vector<int> getBoardIndices(int suit, int face);

	void drawBoard(RenderWindow&);
	void drawHands(RenderWindow&);
};