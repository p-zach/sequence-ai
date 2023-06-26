#pragma once

#include <SFML\Graphics.hpp>
#include "Card.hpp"
#include "Constants.hpp"
#include "SequenceModel.hpp"
#include "GameView.hpp"

#include <set>
#include <vector>

using namespace sf;
using namespace std;

class GameController
{
public:
	GameController();
	void update(RenderWindow&, float);
	void draw(RenderWindow&);

	int clickCard(int x, int y, Card* usedCard);

	int getPlayerIndex() const;
	Card getHandCard(int player, int index) const;
	vector<int> getTokenPositions(int player) const;

	bool needDoubleUpdate() const;

private:
	GameView view;
	SequenceModel model;
};