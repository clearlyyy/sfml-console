#include <SFML/Graphics.hpp>
#include <iostream>

class SFMLConsole {
    private:
    sf::RectangleShape bg;
    sf::RectangleShape input;
    sf::RectangleShape titleBar;

    sf::Vector2f consoleSize = sf::Vector2f(700,700);
    
    float inputHeight = 50;
    float titleBarHeight = 40;

    sf::Font defaultFont;
    sf::Text titleText;


    public:
    
    SFMLConsole() {
        std::cout << "Console has been created" << std::endl;

        bg.setSize(consoleSize);
        bg.setPosition(300, 300);
        bg.setFillColor(sf::Color(56, 56, 56));

        titleBar.setSize(sf::Vector2f(consoleSize.x, titleBarHeight));
        titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
        titleBar.setFillColor(sf::Color(48, 48, 48));

        input.setSize(sf::Vector2f(consoleSize.x, inputHeight));
        input.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y + consoleSize.y));
        input.setFillColor(sf::Color(36, 36, 36));

        defaultFont.loadFromFile("Tektur-Bold.ttf");
        titleText.setString("sfml-console");
        titleText.setFont(defaultFont);
        titleText.setCharacterSize(10);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(sf::Color::White);
        
        titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10, titleBar.getPosition().y + (titleBar.getSize().y/3)));


    }

    void Update() {

    }

    void Draw(sf::RenderWindow& window) {
        window.draw(bg);
        window.draw(input);
        window.draw(titleBar);
        window.draw(titleText);
    }

    

    

};