#include <SFML/Graphics.hpp>
#include <iostream>
#include <deque>

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

        std::string getText() {
            return inputString;
        }

        void clearText() {
            inputString = "";
        }

};

struct LogEntry {
    sf::Text text;
    sf::Color color;
};

class ConsoleLogView {
    private:
    
    std::deque<LogEntry> logEntries;
    sf::Vector2f position;
    sf::Vector2f size;

    float scrollOffset = 0.0f;
    const float SCROLL_SPEED = 20.0f;
    const size_t MAX_LOGS = 1000;
    const float LINE_HEIGHT = 20.0f;

    bool autoScroll = true;

    public:

    ConsoleLogView(sf::Vector2f pos, sf::Vector2f size) : position(pos), size(size) {
        std::cout << "Created Log View with SizeY: " << size.y << std::endl;

        if (size.y < LINE_HEIGHT) size.y = LINE_HEIGHT;
    }
    

    void addLog(sf::Font& font, const std::string& message, sf::Color color) {
        // Create new log entry
        LogEntry newEntry;
        newEntry.text.setFont(font);
        newEntry.text.setString(message);
        newEntry.text.setCharacterSize(16);
        newEntry.text.setFillColor(color);
        newEntry.color = color;

        // Add to deque
        logEntries.push_back(newEntry);

        // Remove oldest if exceeding max
        if (logEntries.size() > MAX_LOGS) {
            logEntries.pop_front();
        }

        // Auto-scroll to bottom if enabled
        if (autoScroll) {
            scrollToBottom();
        }
    }

    void handleScroll(float delta) {
        // User is scrolling - disable auto-scroll
        autoScroll = false;
        
        // Apply scroll (negative delta for natural scrolling)
        scrollOffset -= delta * SCROLL_SPEED;
        
        // Calculate maximum scroll offset
        float contentHeight = logEntries.size() * LINE_HEIGHT;
        float maxScroll = std::max(0.0f, contentHeight - size.y);
        
        // Clamp scroll position
        scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);
        
        // Re-enable auto-scroll if within 1 line of bottom
        if (scrollOffset >= maxScroll - LINE_HEIGHT) {
            autoScroll = true;
        }
    }

    void scrollToBottom() {
        float contentHeight = logEntries.size() * LINE_HEIGHT;
        scrollOffset = std::max(0.0f, contentHeight - size.y);
        autoScroll = true;
    }

    void DrawConsoleLog(sf::RenderWindow& window, sf::RectangleShape background, float inputHeight, float titleBarHeight) {
        // Calculate visible range
        float visibleTop = position.y + titleBarHeight + 5;
        float visibleBottom = visibleTop + size.y;
        std::cout << visibleBottom << " " << background.getSize().y << std::endl;
        float firstVisibleY = visibleTop - scrollOffset;

        std::cout << "Drawing between " << visibleTop << " and " << visibleBottom 
              << " (scroll: " << scrollOffset << ")\n";
        
        // Draw visible logs
        float currentY = firstVisibleY;
        for (const auto& entry : logEntries) {
            // Only draw if within visible area
            if (currentY + LINE_HEIGHT >= visibleTop && currentY <= visibleBottom) {
                // Create a copy to modify position without affecting stored text
                sf::Text text = entry.text;
                text.setPosition(position.x + 10, currentY);
                window.draw(text);
            }
            
            currentY += LINE_HEIGHT;
            
            // Early exit if we're past visible area
            if (currentY > visibleBottom) {
                break;
            }
        }
    }
};

class SFMLConsole {
    private:

    sf::RectangleShape bg;
    sf::RectangleShape input;
    sf::RectangleShape titleBar;

    sf::Vector2f defaultConsolePosition = sf::Vector2f(300, 300);
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
    ConsoleLogView logManager;


    public:
    
    SFMLConsole() 
     : inputObj(defaultFont, sf::Vector2f(100, 200), sf::Vector2f(300, 40)),
       logManager(sf::Vector2f(300,300), sf::Vector2f(consoleSize.x, consoleSize.y - titleBarHeight - 20)) {
        std::cout << "Console has been created" << std::endl;
        logManager.addLog(defaultFont, "Console initialized", sf::Color::Green);
        logManager.addLog(defaultFont, "Console initialized", sf::Color::Green);
        logManager.addLog(defaultFont, "Console initialized", sf::Color::Green);
        logManager.addLog(defaultFont, "Console initialized", sf::Color::Red);

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

            if (event->type == sf::Event::KeyPressed && event->key.code == sf::Keyboard::Enter) {
                std::string enteredText = inputObj.getText();
                logManager.addLog(defaultFont, enteredText, sf::Color::White);
                inputObj.clearText();
            }

            if (event->type == sf::Event::MouseWheelScrolled) {
                logManager.handleScroll(event->mouseWheelScroll.delta);
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
        logManager.DrawConsoleLog(window, bg, inputHeight, titleBarHeight);
        window.draw(titleBar);
        window.draw(titleText);
        window.draw(closeButton);
        inputObj.Draw(window);
    }

};
