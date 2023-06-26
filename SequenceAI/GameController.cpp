#include "GameController.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace sf;

GameController::GameController() : view(*this)
{
    //reset();
}

void GameController::update(RenderWindow& window, float elapsed)
{
    if (!view.isAnimating())
        view.update(window, elapsed);
    else
        view.updateAnimation(elapsed);
}

void GameController::draw(RenderWindow& window)
{
    view.draw(window);
}

int GameController::clickCard(int x, int y, Card* usedCard)
{
    return model.clickCard(x, y, usedCard);
}

int GameController::getPlayerIndex() const
{
    return model.getPlayerIndex();
}

Card GameController::getHandCard(int player, int index) const
{
    return model.getHandCard(player, index);
}

vector<int> GameController::getTokenPositions(int player) const
{
    return model.getTokenPositions(player);
}

bool GameController::needDoubleUpdate() const
{
    return view.needDoubleUpdate();
}