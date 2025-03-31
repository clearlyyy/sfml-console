#include <SFML/Graphics.hpp>

#include "SFMLConsole.hpp"

int main()
{

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML works!", sf::Style::Resize | sf::Style::Close);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    SFMLConsole console;

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


        window.clear(sf::Color(159, 228, 237));
        //window.draw(shape);
        console.Draw(window);
        window.display();
    }

    return 0;
}
