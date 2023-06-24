#pragma once

#include <SFML\Graphics.hpp>
#include "Card.hpp"
#include "Constants.hpp"
#include "SequenceModel.hpp"

#include <set>
#include <vector>

using namespace sf;
using namespace std;

// Forward declaration to prevent circular dependency
class GameController;

class GameView
{
public:
	GameView(GameController&);

	void update(RenderWindow&, float);
	void draw(RenderWindow&);

private:
	GameController& controller;

	Texture cardSheet;
	Texture cardBack;
	Texture tokenTextures[constants::NUM_PLAYERS];
	Sprite cards[constants::NUM_SUITS][constants::NUM_FACES];
	RectangleShape background;
	Font font;

	int highlightedCard;

	Sprite topDiscard;
	void reset();

	void clickCard(int x, int y);

	void loadTexture(Texture&, string);
	void loadContent();

	Vector2f getCardPosition(int x, int y);
	IntRect getCardRect(int x, int y);
	IntRect getHandRect(int player, int index);

	vector<int> getBoardIndices(int suit, int face);
	void highlightSelectedCard(RenderWindow&);
	void checkForCardClick(RenderWindow&);

	void drawBoard(RenderWindow&);
	void drawToken(RenderWindow&, int player, int index);
	void drawHands(RenderWindow&);
	void drawInformation(RenderWindow&);

	void startDiscardAnimation(int player, int index);
	void startDrawCardAnimation(int player, int index);
	void startTokenPlaceAnimation(int player, int x, int y);

	/*void startAnimation(Sprite animated, Vector2f origin, Vector2f destination, GameState stateAfter, bool flip = false);
	void updateAnimation(float);
	float easeInOutCubic(float);*/

	Sprite animated;
	Vector2f animationOrigin;
	Vector2f animationPosition;
	Vector2f animationDestination;
	bool flip;
	float animationTime;
};

