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
    view.update(window, elapsed);
    //if (state == GameState::TURN_P1 || state == GameState::TURN_P2)
    //{
    //    highlightSelectedCard(window);

    //    checkForCardClick(window);
    //}
    //else if (state == GameState::ANIMATING)
    //{
    //    updateAnimation(elapsed);

    //    //if (animated )/*
    //    //{

    //    //}*/
    //}    
}

void GameController::draw(RenderWindow& window)
{
    view.draw(window);
}

int GameController::clickCard(int x, int y)
{
    return model.clickCard(x, y);
}

string GameController::getTurnText() const
{
    return model.getTurnText();
}

Card GameController::getHandCard(int player, int index) const
{
    return model.getHandCard(player, index);
}

set<int> GameController::getTokenPositions(int player) const
{
    return model.getTokenPositions(player);
}