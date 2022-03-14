#pragma once

#include <SFML\Graphics.hpp>
#include "Card.hpp"
#include "Constants.hpp"

#include <set>
#include <vector>

using namespace sf;
using namespace std;

class Game
{
public:
	enum class GameState { TURN_P1, TURN_P2, ANIMATING, WON };

	Game();
	void update(RenderWindow&, float);
	void draw(RenderWindow&);

private:
	GameState state;

	vector<Card> deck;
	Card hands[constants::NUM_PLAYERS][constants::HAND_SIZE];

	Texture cardSheet;
	Texture cardBack;
	Texture tokenTextures[constants::NUM_PLAYERS];
	Sprite cards[constants::NUM_SUITS][constants::NUM_FACES];
	RectangleShape background;
	Font font;

	int highlightedCard;
	set<int> tokenPositions[constants::NUM_PLAYERS];

	Sprite topDiscard;

	void clickCard(int x, int y);

	void loadTexture(Texture&, string);
	void loadContent();
	void reset();
	Card drawCard();

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

	void startAnimation(Sprite animated, Vector2f origin, Vector2f destination, GameState stateAfter, bool flip = false);
	void updateAnimation(float);
	float easeInOutCubic(float);

	Sprite animated;
	Vector2f animationOrigin;
	Vector2f animationPosition;
	Vector2f animationDestination;
	bool flip;
	float animationTime;
	GameState stateAfterAnimation;
};