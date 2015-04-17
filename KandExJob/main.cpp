#include <iostream>
#include <SFML/Graphics.hpp>
#include "Swarm.h"
#include "Utility.h"

#define O false
#define X true

Image img = {
	{ O, O, X, O, O },
	{ O, X, X, X, O },
	{ X, X, X, X, X },
	{ O, X, X, X, O },
	{ O, O, X, O, O },
};

int main(void) {
	std::srand(std::time(0));
	std::cout << "Hello, World!\n";

	sf::RenderWindow window(sf::VideoMode(640, 640), "######");

	Swarm swarm = Swarm(img, 100);
	bool pause = true;
	int numStep = 0;
	window.setTitle("Paused - " + std::to_string(numStep));
	while (window.isOpen())
	{
		sf::Event event;
		bool step = false;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Resized)
				window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code)
				{
				case sf::Keyboard::P:
					pause = !pause;
					if (pause) window.setTitle("Paused - " + std::to_string(numStep));
					else window.setTitle("Running - " + std::to_string(numStep));
					break;
				case sf::Keyboard::S:
					step = true;
					break;
				case sf::Keyboard::G:
					swarm.GradientToggle();
					break;
				case sf::Keyboard::A:
					swarm.AngleToggle();
					break;
				case sf::Keyboard::U:
					swarm.UnitsToggle();
					break;
				case sf::Keyboard::Y:
					swarm.ShadowToggle();
					break;
				case sf::Keyboard::I:
					swarm.IdsToggle();
					break;
				case sf::Keyboard::R:
					pause = true;
					numStep = 0;
					swarm.Reset();
					window.setTitle("Paused - " + std::to_string(numStep));
				default:
					break;
				}
			}
		}
		if (!pause) step = true;
		if (step) {
			swarm.Step();
			numStep++;
			
			if (pause) window.setTitle("Paused - " + std::to_string(numStep));
			else window.setTitle("Running - " + std::to_string(numStep));
		}
		swarm.Draw(window);
	}

	return EXIT_SUCCESS;
}