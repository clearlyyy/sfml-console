#include <SFML/Graphics.hpp>

#include "SFMLConsole.hpp"


void changeBGColor(std::vector<std::string> args, SFMLConsole& console, sf::Color& bgColor) {
	if (args.size() >= 3) {
		int r = std::stoi(args[0]);
		int g = std::stoi(args[1]);
		int b = std::stoi(args[2]);
		console.log("Changed Color to:" + std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b) + ".", sf::Color(r,g,b));
		bgColor = sf::Color(r,g,b);
	}
	else {
		console.log("Command must have 3 parameters: r, g, b. eg. changeColor 255 147 244", sf::Color(163, 75, 72));
	}

}

int main()
{

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML works!", sf::Style::Resize | sf::Style::Close);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    SFMLConsole console(window);
	sf::Color bgColor = sf::Color(159,228,237);


	console.addCommand("changeColor", [&](std::vector<std::string> args) {
        changeBGColor(args, console, bgColor);
    });
	
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            console.Update(&event, window);
        }

        console.Update(nullptr, window);


        window.clear(bgColor);
        //window.draw(shape);
        console.Draw(window);
        window.display();
    }

    return 0;
}
