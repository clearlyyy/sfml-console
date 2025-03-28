#include <SFML/Graphics.hpp>
#include <iostream>

class InputBox {
    private:

        sf::Cursor textCursor;
        sf::Cursor defaultCursor;
        bool cursorLoaded = false;

        sf::Clock m_repeatClock;
        float m_repeatDelay = 0.3f;
        float m_repeatSpeed = 0.05f;
        bool m_keyIsRepeating = false;
        sf::Uint32 m_lastUnicode = 0;

        void processKeyInput(sf::Uint32 unicode) {
            if (unicode == '\b') {
                if (!inputString.empty()) {
                    inputString.pop_back();
                }
            }
            else if (unicode >= 32 && unicode <= 126) {
                inputString += static_cast<char>(unicode);
            }
            text.setString(inputString);
        }

    public:
        sf::RectangleShape box;
        sf::Text text;
        std::string inputString;
        bool isFocused = false;
        bool isHovering = false;


        InputBox(sf::Font& font, sf::Vector2f position, sf::Vector2f size) {

            //Load Cursors
            cursorLoaded = textCursor.loadFromSystem(sf::Cursor::Text) && defaultCursor.loadFromSystem(sf::Cursor::Arrow);

            box.setSize(size);
            box.setFillColor(sf::Color(50, 50, 50));
            box.setOutlineColor(sf::Color::White);
            box.setOutlineThickness(2);
            box.setPosition(position);
            

            text.setFont(font);
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::White);
            text.setPosition(position.x + 10, position.y + (size.y / 2) - 10);
        }

        void handleEvent(const sf::Event& event, sf::RenderWindow& window) {

            //Check if user clicked inside the box;
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
                bool wasFocused = isFocused;
                isFocused = box.getGlobalBounds().contains(mousePos);

            }

            if (isFocused && event.type == sf::Event::TextEntered) {
                processKeyInput(event.text.unicode);
                m_lastUnicode = event.text.unicode;
                m_repeatClock.restart();
                m_keyIsRepeating = false;
            }
        }

        void Update(sf::RenderWindow& window) {
            //Check for Hovering
            if (box.getGlobalBounds().contains(sf::Vector2f(sf::Mouse::getPosition(window))))
                isHovering = true;
            else
                isHovering = false;


            if (cursorLoaded) {
                window.setMouseCursor(isHovering ? textCursor : defaultCursor);
            }

            box.setOutlineThickness(0);

            //Don't do anything else if we arent focused on the input.
            if (!isFocused) return;

            box.setOutlineThickness(1);
            
            //Handle backspace
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace)) {
                float elapsed = m_repeatClock.getElapsedTime().asSeconds();

                if (!m_keyIsRepeating) {
                    if (elapsed > m_repeatDelay) {
                        processKeyInput('\b');
                        m_repeatClock.restart();
                        m_keyIsRepeating = true;
                    }
                }
                else {
                    if (elapsed > m_repeatSpeed) {
                        processKeyInput('\b');
                        m_repeatClock.restart();
                    }
                }
            }
            else {
                m_keyIsRepeating = false;
            }
        }

        void Draw(sf::RenderWindow& window) {
            window.draw(box);
            window.draw(text);
        }

        void setPosition(sf::Vector2f position) {
            box.setPosition(position);
            text.setPosition(position.x + 10, position.y + (box.getSize().y / 2) - 10);
        }

        void setSize(sf::Vector2f size) {
            box.setSize(size);
            text.setPosition(box.getPosition().x + 10, box.getPosition().y + (size.y / 2) - 10);
        }

};

class SFMLConsole {
    private:
    sf::RectangleShape bg;
    sf::RectangleShape input;
    sf::RectangleShape titleBar;

    sf::Vector2f consoleSize = sf::Vector2f(900,700);
    
    float inputHeight = 50;
    float titleBarHeight = 40;

    sf::Font defaultFont;
    sf::Text titleText;
    sf::Text closeButton;

    InputBox inputObj;
    bool m_textInputActive = false;
    
    bool draggingConsole = false;
    sf::Vector2f offset;

    float inputPadding = 10.0f;


    public:
    
    SFMLConsole() 
     : inputObj(defaultFont, sf::Vector2f(100, 200), sf::Vector2f(300, 40)) {
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

        inputObj.setSize(sf::Vector2f(
            input.getSize().x - 2 * inputPadding,
            input.getSize().y - 2 * inputPadding
        ));
        inputObj.setPosition(sf::Vector2f(
            input.getPosition().x + inputPadding,
            input.getPosition().y + inputPadding
        ));

        defaultFont.loadFromFile("Tektur-Bold.ttf");
        titleText.setString("sfml-console");
        titleText.setFont(defaultFont);
        titleText.setCharacterSize(20);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(sf::Color::White);

        closeButton = titleText;
        closeButton.setString("x");
        
        closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
        titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));

    }

    void Update(sf::Event* event, sf::RenderWindow& window) {
        // Check if left mouse button is pressed
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            if (!draggingConsole && titleBar.getGlobalBounds().contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
                // Start dragging and calculate the initial offset
                draggingConsole = true;
                offset = bg.getPosition() - sf::Vector2f(sf::Mouse::getPosition(window));
            }
        } else {
            // Stop dragging when the button is released
            draggingConsole = false;
        }
    
        // Move titleBar if dragging
        if (draggingConsole) {
            bg.setPosition(sf::Vector2f(sf::Mouse::getPosition(window)) + offset);
            titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
            input.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y + consoleSize.y));
            closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
            titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));

            inputObj.setSize(sf::Vector2f(
                input.getSize().x - 2 * inputPadding,
                input.getSize().y - 2 * inputPadding
            ));
            inputObj.setPosition(sf::Vector2f(
                input.getPosition().x + inputPadding,
                input.getPosition().y + inputPadding
            ));

        }

        if (event) {
            inputObj.handleEvent(*event, window);
            
            if (event->type == sf::Event::TextEntered) {
                m_textInputActive = true;
            }
        }

        inputObj.Update(window);

        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Unknown)) {
            m_textInputActive = false;
        }

    }

    void Draw(sf::RenderWindow& window) {
        window.draw(bg);
        window.draw(input);
        window.draw(titleBar);
        window.draw(titleText);
        window.draw(closeButton);
        inputObj.Draw(window);
    }

};
