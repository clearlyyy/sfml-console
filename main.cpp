#include <SFML/Graphics.hpp>
#include <algorithm>

#include "SFMLConsole.hpp"


void changeBGColor(std::vector<std::string> args, sf::Color& bgColor) {
	SFMLConsole& console = SFMLConsole::getInstance();
	if (args.size() >= 3) {
		int r = std::stoi(args[0]);
		int g = std::stoi(args[1]);
		int b = std::stoi(args[2]);
		console.log("Changed Color to:" + std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b) + ".", sf::Color(r,g,b), 24);
		bgColor = sf::Color(r,g,b);
	}
	else {
		console.log("Command must have 3 parameters: r, g, b. eg. changeColor 255 147 244", sf::Color(163, 75, 72));
	}
}

sf::Color HSVtoRGB(float hue, float sat, float val) {
    float c = val * sat;
    float x = c * (1 - std::fabs(std::fmod(hue / 60.0f, 2) - 1));
    float m = val - c;

    float r, g, b;
    if (hue < 60)      { r = c; g = x; b = 0; }
    else if (hue < 120){ r = x; g = c; b = 0; }
    else if (hue < 180){ r = 0; g = c; b = x; }
    else if (hue < 240){ r = 0; g = x; b = c; }
    else if (hue < 300){ r = x; g = 0; b = c; }
    else               { r = c; g = 0; b = x; }

    return sf::Color(
        static_cast<sf::Uint8>((r + m) * 255),
        static_cast<sf::Uint8>((g + m) * 255),
        static_cast<sf::Uint8>((b + m) * 255)
    );
}

void spamConsole(std::vector<std::string> args) {
    SFMLConsole& console = SFMLConsole::getInstance();
	std::srand(std::time(0));
	float hueStep = 360.0f / 10;
    for (int i = 0; i < 10; i++) {
		int randomSize = std::rand() % (25 - 13 + 1) + 13;
        console.log("Welcome to SFML Console, this is a long message that will be wrapped to the next line", HSVtoRGB(i * hueStep, 1.0f, 1.0f), randomSize);
    }
}

int main()
{

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML works!", sf::Style::Resize | sf::Style::Close);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    SFMLConsole& console = SFMLConsole::createInstance(window);
	sf::Color bgColor = sf::Color(159,228,237);

	//fps counter
	sf::Clock clock;
	sf::Clock fpsClock;
	float lastTime = 0;
	sf::Text fpsCounter;
	sf::Font font;
	font.loadFromFile("Tektur-Bold.ttf");
	fpsCounter.setFont(font);
	fpsCounter.setPosition(40,40);
	fpsCounter.setCharacterSize(30);
	fpsCounter.setFillColor(sf::Color::Black);
	//console.addCommand("changeColor", changeBGColor);

	//bgColor needs to be passed in
	console.addCommand("changeColor", [&](std::vector<std::string> args) {
		changeBGColor(args, bgColor);
	});

	console.setSizeY(300);
	console.setBackgroundTransparency(200);

    //No Variables need to be passed in
    console.addCommand("spam", spamConsole);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            console.Update(&event, window);
        }
		float currentTime = clock.restart().asSeconds();
		float fps = 1.f / (currentTime - lastTime);
		if (fpsClock.getElapsedTime().asSeconds() > 0.2) {
			fpsCounter.setString("FPS: " + std::to_string(fps));
			fpsClock.restart();
		}
        window.clear(bgColor);
        //window.draw(shape);
        console.Draw(window);
		window.draw(fpsCounter);
        window.display();
    }

    return 0;
}
