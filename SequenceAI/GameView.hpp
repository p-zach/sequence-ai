#pragma once

#include <SFML\Graphics.hpp>
#include "Card.hpp"
#include "Constants.hpp"
#include "SequenceModel.hpp"

#include <set>
#include <vector>
#include <functional>

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

	bool isAnimating() const;
	void updateAnimation(float);

	bool needDoubleUpdate() const;

	~GameView();

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
	void drawAnimation(RenderWindow&);

	void startDiscardAnimation(int player, int index, int x, int y);
	void startDrawCardAnimation(int player, int index);
	void startTokenPlaceAnimation(int player, int x, int y, int handIndex, bool remove);

	void startAnimation(Sprite animated, Vector2f origin, Vector2f destination, bool flip = false, function<void()> callAfter = nullptr);
	float easeInOutCubic(float);

	Vector2i getDrawPosition();
	Vector2i getDiscardPosition();

	bool currentlyAnimating;
	Sprite animated;
	Vector2f animationOrigin;
	Vector2f animationPosition;
	Vector2f animationDestination;
	bool flip;
	bool drawLastToken;
	int tempTokenPos;
	Card* usedCard;
	int discardPlayer, discardIndex;
	bool drawNewlyDrawnCard;
	float animationTime;
	function<void()> functionAfterAnimation;
};

