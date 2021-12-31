#include "Game.hpp"

Game::Game()
{
	state = TURN_P1;


	// Initialize deck
	for (int i = 0; i < constants::DECK_SIZE; i++)
	{
		deck[i] = Card((i % 52) / 13, (i % 52) % 13);
	}
}

void Game::draw(RenderWindow& window)
{

}