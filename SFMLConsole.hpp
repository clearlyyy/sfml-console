#include <SFML/Graphics.hpp>
#include <iostream>
#include <deque>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <functional>

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

        sf::RectangleShape m_caret;
        sf::Clock m_caretBlinkClock;
        bool m_caretVisible = false;
        const float m_caretBlinkInterval = 0.5f;

        void updateCaretPosition() {
            float caretX = text.findCharacterPos(inputString.size()).x;
            if (inputString.empty()) {
                caretX = text.getPosition().x;
            }
            m_caret.setPosition(caretX, text.getPosition().y);
        }

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
        float textOffset = 0.f; // This will be used for horizontal scrolling
        sf::FloatRect textBounds;

        InputBox(sf::Font& font, sf::Vector2f position, sf::Vector2f size) {
            cursorLoaded = textCursor.loadFromSystem(sf::Cursor::Text) && defaultCursor.loadFromSystem(sf::Cursor::Arrow);

            box.setSize(size);
            box.setFillColor(sf::Color(11, 11, 11));
            box.setPosition(position);
            
            text.setFont(font);
            text.setCharacterSize(16);
            text.setFillColor(sf::Color::White);
            textBounds = text.getLocalBounds();
            text.setOrigin(0, textBounds.top + textBounds.height / 2);
            text.setPosition(position.x + 10, position.y + (size.y / 2));

            m_caret.setSize(sf::Vector2f(2.f, text.getCharacterSize() * 1.1f));
            m_caret.setFillColor(sf::Color::White);
            m_caretVisible = false;
            m_caretBlinkClock.restart();
        }

        void handleEvent(const sf::Event& event, sf::RenderWindow& window) {
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
                bool wasFocused = isFocused;
                isFocused = box.getGlobalBounds().contains(mousePos);

                if (isFocused != wasFocused) {
                    m_caretBlinkClock.restart();
                    m_caretVisible = true;
                }
            }

            if (isFocused && event.type == sf::Event::TextEntered) {
                processKeyInput(event.text.unicode);
                m_lastUnicode = event.text.unicode;
                m_repeatClock.restart();
                m_keyIsRepeating = false;

                updateCaretPosition();
                m_caretBlinkClock.restart();
                m_caretVisible = true;
            }
        }

        void Update(sf::RenderWindow& window) {
            if (box.getGlobalBounds().contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
                isHovering = true;
            } else {
                isHovering = false;
            }

            if (cursorLoaded) {
                window.setMouseCursor(isHovering ? textCursor : defaultCursor);
            }

            if (!isFocused) return;

            box.setFillColor(sf::Color(26, 26, 26));

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

            if (m_caretBlinkClock.getElapsedTime().asSeconds() > m_caretBlinkInterval) {
                m_caretVisible = !m_caretVisible;
                m_caretBlinkClock.restart();
            }

            updateCaretPosition();

            // Get the width of the text
            float textWidth = text.getLocalBounds().width;

            // If the text exceeds the box width, scroll horizontally
            if (textWidth > box.getSize().x - 20) {
                textOffset = textWidth - box.getSize().x + 20; // 20 for padding
            } else {
                textOffset = 0.f;
            }

            // Apply horizontal scrolling to the text
            text.setPosition(box.getPosition().x + 10 - textOffset, box.getPosition().y + (box.getSize().y / 2) - (text.getCharacterSize() / 2));

            // Clip the text rendering if it exceeds the box's left boundary
            if (textOffset > 0.f) {
                text.setPosition(box.getPosition().x + 10 - textOffset, text.getPosition().y);
            }
        }

        //Bit of a hacky way of masking the text so it doesn't overflow from the left of the console when typing a long command.
        void Draw(sf::RenderWindow& window) {
            
            window.draw(box);
    
            sf::RenderTexture maskTexture;
            maskTexture.create(static_cast<unsigned>(box.getSize().x), 
                              static_cast<unsigned>(box.getSize().y));
            maskTexture.clear(sf::Color::Transparent);
        
            sf::Text maskedText = text; 
            maskedText.setPosition(text.getPosition() - box.getPosition());
        
            maskTexture.draw(maskedText);
            maskTexture.display();
        
            sf::Sprite maskSprite(maskTexture.getTexture());
            maskSprite.setPosition(box.getPosition());
            window.draw(maskSprite);
        
            // Draw Caret seperately
            if (isFocused && m_caretVisible) {
                window.draw(m_caret);
            }
        }

        void setPosition(sf::Vector2f position) {
            box.setPosition(position);
            text.setPosition(position.x + 10 - textOffset, position.y + (box.getSize().y / 2) - (text.getCharacterSize() / 2));
        }

        void setSize(sf::Vector2f size) {
            box.setSize(size);
            text.setPosition(box.getPosition().x + 10 - textOffset, box.getPosition().y + (size.y / 2) - 10);
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
    const float PADDING = 10.0f;

    bool autoScroll = true;

    std::vector<std::string> splitLongText(const std::string& message, sf::Font& font, unsigned charSize) {
        std::vector<std::string> lines;
        sf::Text measurer;
        measurer.setFont(font);
        measurer.setCharacterSize(charSize);
        
        float maxWidth = size.x - 2 * PADDING;
        std::string currentLine;
        
        for (char c : message) {
            std::string testLine = currentLine + c;
            measurer.setString(testLine);
            
            // Get the exact position of the last character
            float lineWidth = measurer.findCharacterPos(testLine.size()).x - 
                             measurer.findCharacterPos(0).x;
            
            if (lineWidth > maxWidth) {
                if (!currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = c;  // Start new line with this character
                } else {
                    // Handle case where even a single character is too wide
                    lines.push_back(std::string(1, c));
                    currentLine.clear();
                }
            } else {
                currentLine += c;
            }
        }
        
        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }
        
        return lines;
    }

    public:

    ConsoleLogView(sf::Vector2f pos, sf::Vector2f size) : position(pos), size(size) {
        std::cout << "Created Log View with SizeY: " << size.y << std::endl;

        if (size.y < LINE_HEIGHT) size.y = LINE_HEIGHT;
    }
    
    void addLog(sf::Font& font, const std::string& message, sf::Color color) {
        auto lines = splitLongText(message, font, 16);

        for (const auto& line : lines) {
            LogEntry newEntry;
            newEntry.text.setFont(font);
            newEntry.text.setString(line);
            newEntry.text.setCharacterSize(16);
            newEntry.text.setFillColor(color);
            newEntry.color = color;

            logEntries.push_back(newEntry);
        }

        // Remove oldest if exceeding max
        while (logEntries.size() > MAX_LOGS) {
            logEntries.pop_front();
        }

        // Auto-scroll to bottom if enabled
        if (autoScroll) {
            scrollToBottom();
        }
    }

    void handleScroll(float delta) {
        // user is scrolling, so disable auto scroll
        autoScroll = false;
        
        // Apply scroll
        scrollOffset -= delta * SCROLL_SPEED;
        
        // Calculate maximum scroll offset
        float contentHeight = logEntries.size() * LINE_HEIGHT;
        float maxScroll = std::max(0.0f, contentHeight - size.y);
        
        // Clamp scroll position
        scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);
        
        // Reenable auto-scroll if within 1 line of bottom
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
        float visibleTop = position.y + titleBarHeight;
        float visibleBottom = visibleTop + size.y;
        float firstVisibleY = position.y + titleBarHeight;  // Start drawing from the top of the visible area
        
        // Calculate which lines should be visible based on scroll offset
        size_t firstVisibleLine = static_cast<size_t>(scrollOffset / LINE_HEIGHT);
        float yOffset = firstVisibleY - (scrollOffset - firstVisibleLine * LINE_HEIGHT);
        
        // Draw only visible lines
        for (size_t i = firstVisibleLine; i < logEntries.size(); i++) {
            const auto& entry = logEntries[i];
            sf::Text text = entry.text;
            text.setPosition(position.x + PADDING, yOffset);
            window.draw(text);
            
            yOffset += LINE_HEIGHT;
            
            if (yOffset > visibleBottom) {
                break;
            }
        }
    }

    void setPosition(sf::Vector2f pos) {
        position = pos;
    }

    void setSize(sf::Vector2f sz) {
        size = sz;
    }

    void clear() {
        logEntries.clear();
    }

};

/// @brief Handles and stores commands.
class CommandManager {
    public:
        std::map<std::string, std::function<void()>> commands;

        std::map<std::string, std::function<void()>> getCommandsList() {
            return commands;
        }
        
        void addCommand(const std::string& name, std::function<void()> func) {
            commands[name] = func;
        }
};

class SFMLConsole {
    private:

    CommandManager cmdManager;

    sf::RectangleShape bg;
    sf::RectangleShape titleBar;

    sf::Vector2f defaultConsolePosition = sf::Vector2f(300, 300);
    sf::Vector2f consoleSize = sf::Vector2f(900,500);

    float inputHeight = 30;
    float titleBarHeight = 30;

    sf::Font defaultFont;
    sf::Text titleText;
    sf::Text closeButton;

    InputBox inputObj;
    bool m_textInputActive = false;
    
    bool draggingConsole = false;
    sf::Vector2f offset;

    float inputPadding = 10.0f;
    ConsoleLogView logManager;

    void executeCommand(const std::string& name) {
        std::map<std::string, std::function<void()>> cmds = cmdManager.getCommandsList();
        auto it = cmds.find(name);
        if (it != cmds.end()) {
            it->second();
        } else {
            logManager.addLog(defaultFont, "Command not found, type 'help' to see a list of avaliable commands.", sf::Color(227, 100, 100) );
        }
    }

    void displayCommands() {
        std::map<std::string, std::function<void()>> cmds = cmdManager.getCommandsList();
        logManager.addLog(defaultFont, "------All Currently Avaliable Commands------", sf::Color::White);
        for (const auto& pair : cmds) {
            logManager.addLog(defaultFont, pair.first, sf::Color::White);
        }
    }

    public:
    
    // Constructor 
    SFMLConsole() 
     : inputObj(defaultFont, sf::Vector2f(100, 200), sf::Vector2f(300, 40)),
       logManager(defaultConsolePosition, sf::Vector2f(consoleSize.x, consoleSize.y - titleBarHeight - inputHeight - 5)) {
        

        bg.setSize(consoleSize);
        bg.setPosition(defaultConsolePosition);
        bg.setFillColor(sf::Color(56, 56, 56));

        bg.setOutlineColor(sf::Color(26, 26, 26));
        bg.setOutlineThickness(2);

        titleBar.setSize(sf::Vector2f(consoleSize.x, titleBarHeight));
        titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
        titleBar.setFillColor(sf::Color(48, 48, 48));

        inputObj.setSize(sf::Vector2f(
            bg.getSize().x, //- 2 * inputPadding,
            inputHeight //- 2 * inputPadding
        ));
        inputObj.setPosition(sf::Vector2f(
            bg.getPosition().x, //+ inputPadding,
            bg.getPosition().y + consoleSize.y - inputHeight //+ inputPadding
        ));

        if (!defaultFont.loadFromFile("Terminal F4.ttf")) {
            std::cout << "SFML-CONSOLE: ERROR LOADING defaultFont << std::endl" << std::endl;
        }

        titleText.setString("SFML-CONSOLE");
        titleText.setFont(defaultFont);
        titleText.setCharacterSize(16);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(sf::Color::White);

        closeButton = titleText;
        closeButton.setString("x");
        
        closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
        titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));

        logManager.addLog(defaultFont, "Console initialized gjfdso ghjfsdkjghfkjdsghfdksjghfdjkshgfdkljshgfkdlshgkfd kjsfdjkghfsdjkghfdjk  ghfkdjsgh kfdshg jkfdshgkjfdhs kjgfdkjgsdfkj ghfdksjg hfkjdshgkfjdsh gfkdshgkfdshgdfslgkj", sf::Color::Green);
        logManager.addLog(defaultFont, "Console initialized", sf::Color::Green);
        logManager.addLog(defaultFont, "Console initialized", sf::Color::Green);
        logManager.addLog(defaultFont, "Console initialized", sf::Color::Red);

        cmdManager.addCommand("clear", std::bind(&ConsoleLogView::clear, &logManager));
        cmdManager.addCommand("help", std::bind(&SFMLConsole::displayCommands, this));

    }

    // Update Function, place this inside your event loop, otherwise use a nullptr for event.
    void Update(sf::Event* event, sf::RenderWindow& window) {
        // Check if left mouse button is pressed
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            if (!draggingConsole && titleBar.getGlobalBounds().contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
                // Start dragging and calculate the initial offset
                draggingConsole = true;
                offset = bg.getPosition() - sf::Vector2f(sf::Mouse::getPosition(window));
            }
        } else {
            // Stop dragging when left mouse is released
            draggingConsole = false;
        }
    
        // Move titleBar if dragging
        // Probably will simplify this as its pretty fucked.
        if (draggingConsole) {
            bg.setPosition(sf::Vector2f(sf::Mouse::getPosition(window)) + offset);
            titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
            //input.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y + consoleSize.y));
            closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
            titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));
            logManager.setPosition(bg.getPosition());

            inputObj.setSize(sf::Vector2f(
                bg.getSize().x, //- 2 * inputPadding,
                inputHeight //- 2 * inputPadding
            ));
            inputObj.setPosition(sf::Vector2f(
                bg.getPosition().x, //+ inputPadding,
                bg.getPosition().y + consoleSize.y - inputHeight //+ inputPadding
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

                //Try to run command.
                executeCommand(enteredText);

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


    // Draw the Console, place this at the very top of your scene, otherwise your game may be drawn over it.
    void Draw(sf::RenderWindow& window) {
        window.draw(bg);
        logManager.DrawConsoleLog(window, bg, inputHeight, titleBarHeight);
        window.draw(titleBar);
        window.draw(titleText);
        window.draw(closeButton);
        inputObj.Draw(window);  
    }

    
    // Public CMD Functions
    void addCommand(std::string cmd, std::function<void()> func) {
        cmdManager.addCommand(cmd, func);
    }
    
};
