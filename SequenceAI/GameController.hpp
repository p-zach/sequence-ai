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

	int clickCard(int x, int y);

	string getTurnText() const;
	Card getHandCard(int player, int index) const;
	set<int> getTokenPositions(int player) const;

private:
	GameView view;
	SequenceModel model;
};