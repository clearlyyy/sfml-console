// Clearly
// SFML-CONSOLE - Header only Console Library for SFML.
// For more information or help, checkout the documentation at https://github.com/clearlyyy/sfml-console

#pragma once


#include <SFML/Graphics.hpp>
#include <iostream>
#include <deque>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <limits>
#include <tgmath.h>

class InputBox {
    private:
	std::deque<std::string> pastInputs;
	int currentPastInputIndex = 0;
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
    size_t m_caretPosition = 0;

    size_t m_selectionStart = 0;
    size_t m_selectionEnd = 0;
    size_t m_selectionAnchor = 0;
    bool m_isSelecting = false;
    sf::RectangleShape m_selectionHighlight;
    void updateCaretPosition() {
        float caretX = text.getPosition().x;
        if (!inputString.empty()) {
            if (m_caretPosition > 0) {
                caretX = text.findCharacterPos(m_caretPosition).x;
            }
        }
        m_caret.setPosition(caretX, text.getPosition().y);
    }

    void processKeyInput(sf::Uint32 unicode) {
        if (hasSelection() && (unicode == '\b' || unicode == 127)) {
            deleteSelectedText();
            return;
        }
        if (unicode == '\b') { // Backspace Key
            if (m_caretPosition > 0 && !inputString.empty()) {
                inputString.erase(m_caretPosition - 1, 1);
                m_caretPosition--;
            }
        } else if (unicode == 127) { // Delete Key
            if (m_caretPosition < inputString.size()) {
                inputString.erase(m_caretPosition, 1);
            }
        } else if (unicode >= 32 && unicode <= 126) {
            inputString.insert(m_caretPosition, 1, static_cast<char>(unicode));
            m_caretPosition++;
        }
        text.setString(inputString);
    }
    size_t findClickedCharacterPosition(float mouseX) {
        // If the string is empty or mouse is before the first character, return 0
        if (inputString.empty() || mouseX <= 0) {
            return 0;
        }
    
        // Get the global bounds of the visible text
        float textLeft = text.getPosition().x;
        float textRight = textLeft + text.getLocalBounds().width;
    
        // If mouse is beyond the text right edge, return end position
        if (mouseX >= textRight - textLeft) {
            return inputString.size();
        }
    
        // Binary search to find the closest character position
        size_t low = 0;
        size_t high = inputString.size();
        size_t bestMatch = inputString.size();
        float smallestDistance = std::numeric_limits<float>::max();
    
        while (low <= high) {
            size_t mid = low + (high - low) / 2;
            float charPos = text.findCharacterPos(mid).x - textLeft;
    
            float distance = std::abs(mouseX - charPos);
            if (distance < smallestDistance) {
                smallestDistance = distance;
                bestMatch = mid;
            }
    
            if (charPos < mouseX) {
                low = mid + 1;
            } else if (charPos > mouseX) {
                high = mid - 1;
            } else {
                // Exact match found
                return mid;
            }
        }
    
        // For the best match, check if we should be before or after the character
        if (bestMatch > 0) {
            float prevCharPos = text.findCharacterPos(bestMatch - 1).x - textLeft;
            float currCharPos = text.findCharacterPos(bestMatch).x - textLeft;
            float midpoint = prevCharPos + (currCharPos - prevCharPos) / 2;
            
            if (mouseX < midpoint) {
                return bestMatch - 1;
            }
        }
    
        return bestMatch;
    }
    void copyToClipboard() {
        if (hasSelection()) {
            size_t start = std::min(m_selectionStart, m_selectionEnd);
            size_t end = std::max(m_selectionStart, m_selectionEnd);
            std::string selectedText = inputString.substr(start, end - start);
            sf::Clipboard::setString(selectedText);
        }
    }
    
    void cutToClipboard() {
        if (hasSelection()) {
            copyToClipboard();
            deleteSelectedText();
        }
    }
    
    void pasteFromClipboard() {
        std::string clipboardText = sf::Clipboard::getString();
        if (!clipboardText.empty()) {
            if (hasSelection()) {
                deleteSelectedText();
            }
            inputString.insert(m_caretPosition, clipboardText);
            m_caretPosition += clipboardText.size();
            text.setString(inputString);
        }
    }
    
    void deleteSelectedText() {
        if (hasSelection()) {
            size_t start = std::min(m_selectionStart, m_selectionEnd);
            size_t end = std::max(m_selectionStart, m_selectionEnd);
            inputString.erase(start, end - start);
            m_caretPosition = start;
            text.setString(inputString);
            clearSelection();
        }
    }
    
    bool hasSelection() const {
        return m_selectionStart != m_selectionEnd;
    }
    
    void clearSelection() {
        m_selectionStart = m_caretPosition;
        m_selectionEnd = m_caretPosition;
        m_selectionAnchor = m_caretPosition;
    }
    void updateSelectionHighlight() {
        if (!hasSelection()) {
            return;
        }
    
        // Get positions of selection start and end
        float startX = text.findCharacterPos(m_selectionStart).x;
        float endX = text.findCharacterPos(m_selectionEnd).x;
    
        // Init the highlight rectangle
        m_selectionHighlight.setPosition(startX, text.getPosition().y);
        m_selectionHighlight.setSize(sf::Vector2f(endX - startX, text.getCharacterSize() * 1.2f));
    }

    public:
    sf::RectangleShape box;
    sf::Text text;
    std::string inputString;
    bool isFocused = false;
    bool isHovering = false;
    float textOffset = 0.f; 
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
        m_selectionHighlight.setFillColor(sf::Color(100,100,255, 150));
    }
    void handleEvent(const sf::Event& event, sf::RenderWindow& window) {
        // Handle focus changes
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            bool wasFocused = isFocused;
            isFocused = box.getGlobalBounds().contains(mousePos);
    
            if (isFocused) {
                // Just gained focus
                if (!wasFocused) {
                    m_caretBlinkClock.restart();
                    m_caretVisible = true;
                }
                
                // Handle text selection/caret positioning
                float mouseX = mousePos.x - text.getPosition().x + textOffset;
                m_caretPosition = findClickedCharacterPosition(mouseX);
                
                // Initialize selection - track initial position
                m_selectionStart = m_caretPosition;
                m_selectionEnd = m_caretPosition;
				m_selectionAnchor = m_caretPosition;
                m_isSelecting = true;
                
                return;
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            m_isSelecting = false;
        }
    
        // Only process these events if we have focus
        if (!isFocused) return;
    
        // Handle text selection dragging
        if (event.type == sf::Event::MouseMoved && m_isSelecting) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            float mouseX = mousePos.x - text.getPosition().x + textOffset;
            size_t newCaretPos = findClickedCharacterPosition(mouseX);
            
            // Update caret position
            m_caretPosition = newCaretPos;
            
            // Update selection - maintain original anchor point
            if (newCaretPos < m_selectionStart) {
                // Dragging left
                m_selectionStart = newCaretPos;
				m_selectionEnd = m_selectionAnchor;
            } 
            else {
                // Dragging right
				m_selectionStart = m_selectionAnchor;
                m_selectionEnd = newCaretPos;
            }
        }
    
        // Handle Keyboard shortcuts
        if (event.type == sf::Event::KeyPressed) {
            bool ctrlPressed = event.key.control;
            bool shiftPressed = event.key.shift;
            if (ctrlPressed) {
                if (event.key.code == sf::Keyboard::C && hasSelection()) {
                    copyToClipboard();
                    return;
                }
                else if (event.key.code == sf::Keyboard::X && hasSelection()) {
                    cutToClipboard();
                    return;
                }
                else if (event.key.code == sf::Keyboard::V) {
                    pasteFromClipboard();
                    return;
                }
            }
            if (ctrlPressed && event.key.code == sf::Keyboard::A) {
                // Select all text
                m_selectionStart = 0;
                m_selectionEnd = inputString.size();
                m_caretPosition = inputString.size(); 
                m_selectionAnchor = 0; 
                m_caretBlinkClock.restart();
                m_caretVisible = true;
                return; 
            }
            // Handle navigation keys with selection
            if (event.key.code == sf::Keyboard::Left) {
                if (m_caretPosition > 0) {
                    m_caretPosition--;
                    if (shiftPressed) {
                        if (!hasSelection()) {
                            // Start new selection - anchor at original position
                            m_selectionAnchor = m_caretPosition + 1;
                            m_selectionStart = m_selectionAnchor;
                            m_selectionEnd = m_caretPosition;
                        } else {
                            // Extend existing selection
                            if (m_caretPosition < m_selectionAnchor) {
                                m_selectionStart = m_caretPosition;
                                m_selectionEnd = m_selectionAnchor;
                            } else {
                                m_selectionStart = m_selectionAnchor;
                                m_selectionEnd = m_caretPosition;
                            }
                        }
                    } else {
                        clearSelection();
                    }
                }
                m_caretBlinkClock.restart();
                m_caretVisible = true;
            } 
            else if (event.key.code == sf::Keyboard::Right) {
                if (m_caretPosition < inputString.size()) {
                    m_caretPosition++;
                    if (shiftPressed) {
                        if (!hasSelection()) {
                            m_selectionAnchor = m_caretPosition - 1;
                            m_selectionStart = m_selectionAnchor;
                            m_selectionEnd = m_caretPosition;
                        } else {
                            if (m_caretPosition > m_selectionAnchor) {
                                m_selectionStart = m_selectionAnchor;
                                m_selectionEnd = m_caretPosition;
                            } else {
                                m_selectionStart = m_caretPosition;
                                m_selectionEnd = m_selectionAnchor;
                            }
                        }
                    } else {
                        clearSelection();
                    }
                }
                m_caretBlinkClock.restart();
                m_caretVisible = true;
            }
			else if (event.key.code == sf::Keyboard::Up) {
				if (currentPastInputIndex < pastInputs.size() && pastInputs.size() > 1) {
					inputString = pastInputs[currentPastInputIndex];
					text.setString(pastInputs[currentPastInputIndex]);
					currentPastInputIndex++;
				}
				
			}
			else if (event.key.code == sf::Keyboard::Down) {
				if (currentPastInputIndex < pastInputs.size() && pastInputs.size() > 1) {
					inputString = pastInputs[currentPastInputIndex];
					text.setString(pastInputs[currentPastInputIndex]);
				}
				currentPastInputIndex--;
				if (currentPastInputIndex < 0) {
					currentPastInputIndex = 0;
				}
			}
				
            else if (event.key.code == sf::Keyboard::Delete) {
                if (hasSelection()) {
                    deleteSelectedText();
                } else if (m_caretPosition < inputString.size()) {
                    inputString.erase(m_caretPosition, 1);
                    text.setString(inputString);
                }
            }
        }
    
        // Handle text input (separate from key presses)
        if (event.type == sf::Event::TextEntered) {
            // Ignore control characters (except backspace which is handled elsewhere)
            if (event.text.unicode == '\b') {
                if (hasSelection()) {
                    deleteSelectedText();
                } else if (m_caretPosition > 0) {
                    inputString.erase(m_caretPosition - 1, 1);
                    m_caretPosition--;
                    text.setString(inputString);
                }
                m_lastUnicode = event.text.unicode;
                m_repeatClock.restart();
                m_keyIsRepeating = false;
    
                updateCaretPosition();
                m_caretBlinkClock.restart();
                m_caretVisible = true;
            }
            else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                if (hasSelection()) {
                    deleteSelectedText();
                }
                inputString.insert(m_caretPosition, 1, static_cast<char>(event.text.unicode));
                m_caretPosition++;
                text.setString(inputString);
                
                m_lastUnicode = event.text.unicode;
                m_repeatClock.restart();
                m_keyIsRepeating = false;
    
                updateCaretPosition();
                m_caretBlinkClock.restart();
                m_caretVisible = true;
            }
        }
    }

    bool isHoveringFlag() {
        return isHovering;
    }

    void Update(sf::RenderWindow& window) {
        isHovering = box.getGlobalBounds().contains(sf::Vector2f(sf::Mouse::getPosition(window)));
        if (cursorLoaded && isHovering) {
            window.setMouseCursor(textCursor);
        }
        
        if (!isFocused) return;
        updateSelectionHighlight();
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
        if (isFocused && hasSelection()) {
            window.draw(m_selectionHighlight);
        }

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
        m_caretPosition = 0;
        text.setString(inputString);
        updateCaretPosition();
    }

	void addToPastInputs(std::string msg) {
		pastInputs.push_front(msg);
		if (pastInputs.size() > 30) {
			pastInputs.pop_back();
		}
	}

	void setCurrentPastInputsIndex(int num) {
		currentPastInputIndex = 0;
	}
        
};

class ConsoleLogView {
    private:
        struct LogEntry {
            std::string originalMessage;
            sf::Color color;
            float charSize;
            std::vector<std::string> wrappedLines;
            float lastWrapWidth = 0;
        };
    
        struct TextBatch {
            sf::VertexArray vertices;
            const sf::Texture* texture;
            unsigned int fontSize;
        };
    
        float contentHeight;
        std::deque<LogEntry> logEntries;
        std::vector<TextBatch> batchedTexts;
        sf::Vector2f position;
        sf::Vector2f size;
        float scrollOffset = 0.0f;
        const float SCROLL_SPEED = 40.0f;
        size_t MAX_LOGS = 500;
        const float LINE_HEIGHT = 20.0f;
        const float PADDING = 10.0f;
        bool autoScroll = true;
        float lastWrapWidth = 0;
        bool needsRedraw = true;
        size_t totalLines = 0;
        const float TOP_MARGIN = 10.0f;
        
        // Optimization parameters
        const unsigned int MAX_FONT_SIZES = 8;
        const unsigned int COLOR_QUANTIZATION = 16;
    
        sf::Color quantizeColor(const sf::Color& color, unsigned int levels) {
            if (levels == 0) levels = 1;
            float step = 255.0f / (levels - 1);
            return sf::Color(
                static_cast<sf::Uint8>(std::round(color.r / step) * step),
                static_cast<sf::Uint8>(std::round(color.g / step) * step),
                static_cast<sf::Uint8>(std::round(color.b / step) * step),
                color.a
            );
        }
    
        unsigned int roundFontSize(unsigned int size) {
            // Round to nearest multiple of 4 to reduce texture variations
            return ((size + 2) / 4) * 4;
        }
    
        std::vector<std::string> wrapText(const std::string& message, sf::Font& font, float maxWidth, float charSize) {
            std::vector<std::string> lines;
            if (message.empty() || maxWidth <= 0) return lines;
    
            const sf::Glyph& spaceGlyph = font.getGlyph(' ', charSize, false);
            float spaceAdvance = spaceGlyph.advance;
            float currentLineWidth = 0;
            size_t lineStart = 0;
            size_t lastSpace = std::string::npos;
    
            for (size_t i = 0; i < message.length(); i++) {
                char c = message[i];
                const sf::Glyph& glyph = font.getGlyph(c, charSize, false);
                float charWidth = glyph.advance;
    
                if (c == ' ' || c == '\t') {
                    lastSpace = i;
                }
    
                if (currentLineWidth + charWidth > maxWidth) {
                    if (lastSpace != std::string::npos && lastSpace > lineStart) {
                        lines.push_back(message.substr(lineStart, lastSpace - lineStart));
                        lineStart = lastSpace + 1;
                        i = lineStart;
                        lastSpace = std::string::npos;
                        currentLineWidth = 0;
                    } else {
                        lines.push_back(message.substr(lineStart, i - lineStart));
                        lineStart = i;
                        lastSpace = std::string::npos;
                        currentLineWidth = charWidth;
                    }
                } else {
                    currentLineWidth += charWidth;
                }
            }
    
            if (lineStart < message.length()) {
                lines.push_back(message.substr(lineStart));
            }
    
            return lines;
        }
    
        void regenerateVertexArray(sf::Font& font) {
            batchedTexts.clear();
            std::map<unsigned int, TextBatch> sizeBatches;
    
            float maxWidth = size.x - 2 * PADDING;
            float yOffset = position.y + TOP_MARGIN + 16;
            float currentScroll = scrollOffset;
    
            contentHeight = 0.0f;
    
            // First pass: calculate full content height
            for (const auto& entry : logEntries) {
                for (const auto& line : entry.wrappedLines) {
                    sf::Text tempText;
                    tempText.setFont(font);
                    tempText.setString(line);
                    tempText.setCharacterSize(entry.charSize);
                    float lineHeight = tempText.getLocalBounds().height + 4;
                    contentHeight += lineHeight;
                }
            }
    
            contentHeight += 20;
    
            if (autoScroll) {
                float maxScroll = std::max(0.0f, contentHeight - size.y + TOP_MARGIN);
                scrollOffset = maxScroll;
            }
    
            yOffset = position.y + TOP_MARGIN + 16;
            currentScroll = scrollOffset;
    
            // Second pass: generate batched vertices
            for (const auto& entry : logEntries) {
                unsigned int roundedSize = roundFontSize(entry.charSize);
                sf::Color quantColor = quantizeColor(entry.color, COLOR_QUANTIZATION);
    
                for (const auto& line : entry.wrappedLines) {
                    sf::Text tempText;
                    tempText.setFont(font);
                    tempText.setString(line);
                    tempText.setCharacterSize(entry.charSize);
                    float lineHeight = tempText.getLocalBounds().height + 4;
    
                    if (currentScroll > lineHeight) {
                        currentScroll -= lineHeight;
                        continue;
                    }
    
                    if (yOffset > position.y + size.y) {
                        break;
                    }
    
                    // Get or create batch for this font size
                    auto& batch = sizeBatches[roundedSize];
                    if (batch.vertices.getVertexCount() == 0) {
                        batch.vertices.setPrimitiveType(sf::Quads);
                        batch.texture = &font.getTexture(roundedSize);
                        batch.fontSize = roundedSize;
                    }
    
                    // Create text for positioning
                    sf::Text text;
                    text.setFont(font);
                    text.setString(line);
                    text.setCharacterSize(entry.charSize);
                    text.setFillColor(quantColor);
                    text.setPosition(position.x + PADDING, yOffset - currentScroll);
    
                    // Convert to UTF-32
                    sf::String sfmlString = sf::String::fromUtf8(line.begin(), line.end());
                    const sf::Uint32* characters = sfmlString.getData();
                    const std::size_t length = sfmlString.getSize();
    
                    const sf::Transform transform(text.getTransform());
                    const sf::Uint32 style(text.getStyle());
                    const float outlineThickness(text.getOutlineThickness());
    
                    float x = 0.f;
                    float y = static_cast<float>(entry.charSize);
    
                    for (std::size_t i = 0; i < length; ++i) {
                        sf::Uint32 currentChar = characters[i];
                        
                        if (currentChar == '\r') continue;
                        
                        if (currentChar == '\t') {
                            const sf::Glyph& glyph = font.getGlyph(' ', entry.charSize, style & sf::Text::Bold, outlineThickness);
                            x += glyph.advance * 4;
                            continue;
                        }
                        
                        const sf::Glyph& glyph = font.getGlyph(currentChar, entry.charSize, style & sf::Text::Bold, outlineThickness);
                        
                        float left = glyph.bounds.left;
                        float top = glyph.bounds.top;
                        float right = glyph.bounds.left + glyph.bounds.width;
                        float bottom = glyph.bounds.top + glyph.bounds.height;
                        
                        batch.vertices.append(sf::Vertex(
                            transform.transformPoint(x + left, y + top),
                            quantColor,
                            sf::Vector2f(glyph.textureRect.left, glyph.textureRect.top)
                        ));
                        batch.vertices.append(sf::Vertex(
                            transform.transformPoint(x + right, y + top),
                            quantColor,
                            sf::Vector2f(glyph.textureRect.left + glyph.textureRect.width, glyph.textureRect.top)
                        ));
                        batch.vertices.append(sf::Vertex(
                            transform.transformPoint(x + right, y + bottom),
                            quantColor,
                            sf::Vector2f(glyph.textureRect.left + glyph.textureRect.width, glyph.textureRect.top + glyph.textureRect.height)
                        ));
                        batch.vertices.append(sf::Vertex(
                            transform.transformPoint(x + left, y + bottom),
                            quantColor,
                            sf::Vector2f(glyph.textureRect.left, glyph.textureRect.top + glyph.textureRect.height)
                        ));
                        
                        x += glyph.advance;
                    }
    
                    yOffset += lineHeight;
                    currentScroll = 0;
                }
            }
    
            // Transfer batches to main vector
            for (auto& pair : sizeBatches) {
                if (pair.second.vertices.getVertexCount() > 0) {
                    batchedTexts.push_back(std::move(pair.second));
                }
            }
    
            needsRedraw = false;
        }
    
    public:
        ConsoleLogView(sf::Vector2f pos, sf::Vector2f sz) : position(pos), size(sz) {
            if (size.y < LINE_HEIGHT) size.y = LINE_HEIGHT;
        }
    
        void handleResize(sf::Font& font) {
            float newWidth = size.x;
            if (std::abs(lastWrapWidth - newWidth) < 1.0f) return;
    
            lastWrapWidth = newWidth;
            float maxWidth = newWidth - 2 * PADDING;
            totalLines = 0;
    
            for (auto& entry : logEntries) {
                if (std::abs(entry.lastWrapWidth - maxWidth) > 1.0f) {
                    entry.wrappedLines = wrapText(entry.originalMessage, font, maxWidth, entry.charSize);
                    entry.lastWrapWidth = maxWidth;
                }
                totalLines += entry.wrappedLines.size();
            }
    
            needsRedraw = true;
            if (autoScroll) {
                scrollToBottom();
            }
        }
    
        void addLog(sf::Font& font, const std::string& message, sf::Color color, float charSize = 16) {
            float maxWidth = size.x - 2 * PADDING;
            LogEntry newEntry;
            newEntry.originalMessage = message;
            newEntry.color = quantizeColor(color, COLOR_QUANTIZATION);
            newEntry.charSize = roundFontSize(charSize);
            newEntry.wrappedLines = wrapText(message, font, maxWidth, newEntry.charSize);
            newEntry.lastWrapWidth = maxWidth;
            logEntries.push_back(newEntry);
            totalLines += newEntry.wrappedLines.size();
    
            while (logEntries.size() > MAX_LOGS) {
                totalLines -= logEntries.front().wrappedLines.size();
                logEntries.pop_front();
            }
    
            needsRedraw = true;
        }
    
        void handleScroll(float delta) {
            autoScroll = false;
            scrollOffset -= delta * SCROLL_SPEED;
            float maxScroll = std::max(0.0f, contentHeight - size.y + TOP_MARGIN);
            scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);
            
            if (scrollOffset >= maxScroll - LINE_HEIGHT) {
                autoScroll = true;
            }
    
            needsRedraw = true;
        }
    
        void scrollToBottom() {
            float maxScroll = std::max(0.0f, contentHeight - size.y + TOP_MARGIN);
            scrollOffset = maxScroll;
            autoScroll = true;
            needsRedraw = true;
        }
    
        void DrawConsoleLog(sf::RenderWindow& window, sf::Font& font, sf::RectangleShape background, float inputHeight, float titleBarHeight) {
            if (needsRedraw) {
                regenerateVertexArray(font);
            }
    
            static sf::RenderStates states;
            for (const auto& batch : batchedTexts) {
                states.texture = batch.texture;
                window.draw(batch.vertices, states);
            }
        }
    
        void setPosition(sf::Vector2f pos) { 
            position = pos; 
            needsRedraw = true;
        }
    
        void setSize(sf::Vector2f sz) { 
            size = sz; 
            if (size.y < LINE_HEIGHT) size.y = LINE_HEIGHT;
            needsRedraw = true;
        }
    
        void clear() { 
            logEntries.clear(); 
            batchedTexts.clear();
            scrollOffset = 0;
            totalLines = 0;
            needsRedraw = true;
            autoScroll = true;
        }
    
        void setMaxLogs(size_t maxLogs) {
            MAX_LOGS = maxLogs;
        }
    };

/// @brief Handles and stores commands.
class CommandManager {
    public:
	//Average c++ definition
	std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands;

	std::map<std::string, std::function<void(const std::vector<std::string>&)>>& getCommandsList() {
            return commands;
    }
        
	void addCommand(const std::string& name, std::function<void(const std::vector<std::string>&)> func) {
            commands[name] = func;
    }
};

class SFMLConsole {
    private:

	enum resizeType {
		NONE,
		
		LEFT_EDGE,
		RIGHT_EDGE,
		TOP_EDGE,
		BOTTOM_EDGE,

		TOPLEFT_CORNER,
		TOPRIGHT_CORNER,
		BOTTOMLEFT_CORNER,
		BOTTOMRIGHT_CORNER
	};

	static SFMLConsole* instance;

	std::string titleStr = "SFML-CONSOLE";
	sf::Cursor arrowCursor, hResizeCursor, vResizeCursor, diagResize1Cursor, diagResize2Cursor;
    resizeType activeResizeRegion = resizeType::NONE;
    sf::Vector2f resizeStartMousePos;
    sf::Vector2f resizeStartConsolePos;
    sf::Vector2f resizeStartConsoleSize;

	sf::RenderWindow* window = nullptr;

    sf::Color backgroundColor = sf::Color(44,44,44);
	
	bool floating = true;
	
	sf::Keyboard::Key openConsoleKey = sf::Keyboard::Tilde;
	sf::Clock openConsoleClock;
	bool didConsoleJustOpen = false;
    bool isVisible = true;
    CommandManager cmdManager;

    sf::RectangleShape bg;
    sf::RectangleShape titleBar;

    sf::Vector2f defaultConsolePosition = sf::Vector2f(300, 300);
    sf::Vector2f consoleSize = sf::Vector2f(900,500);
    sf::Vector2f position;

    float inputHeight = 30;
    float titleBarHeight = 30;

    sf::Font defaultFont;
    sf::Text titleText;
    sf::Text closeButton;

    InputBox inputObj;
    bool m_textInputActive = false;

	bool resizingConsole = false;
    bool draggingConsole = false;
	float resizeZonePadding = 5; 
    sf::Vector2f offset;

    float inputPadding = 10.0f;
    ConsoleLogView logManager;
   
    void executeCommand(const std::string& name, const std::vector<std::string>& args) {
        const auto& cmds = cmdManager.getCommandsList();
        if (cmds.find(name) != cmds.end()) {
			std::cout << "Command Found: " << name << std::endl;
			cmds.at(name)(args);
        }        
        else {
            logManager.addLog(defaultFont, "Command not found, type 'help' to see a list of available commands.", sf::Color(227, 100, 100) );
        }
    }

    // Display all available commands
    void displayCommands() {
        std::map<std::string, std::function<void(const std::vector<std::string>&)>> cmds = cmdManager.getCommandsList();
        logManager.addLog(defaultFont, "------All Currently Available Commands------", sf::Color::Green, 20);
        for (const auto& pair : cmds) {
            logManager.addLog(defaultFont, pair.first, sf::Color::White);
        }
    }

	// Constructor 
    SFMLConsole(sf::RenderWindow &window, bool disableStartupLogs = false, bool floating = true) 
     : inputObj(defaultFont, sf::Vector2f(100, 200), sf::Vector2f(300, 40)),
       logManager(defaultConsolePosition, sf::Vector2f(consoleSize.x, consoleSize.y - titleBarHeight - inputHeight - 5)) {

		this->floating = floating;

		if (!floating) {
			consoleSize = sf::Vector2f(window.getSize().x, consoleSize.y);
			defaultConsolePosition = sf::Vector2f(0,0);	
			logManager.setSize(sf::Vector2f(window.getSize().x, consoleSize.y));
            backgroundColor = sf::Color(56,56,56,148);
		}

        bg.setFillColor(backgroundColor);
		logManager.setPosition(defaultConsolePosition);
		openConsoleClock.restart();
        bg.setSize(consoleSize);
        bg.setPosition(defaultConsolePosition);
        position = defaultConsolePosition;
        
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

        if (!defaultFont.loadFromFile("Tektur-Bold.ttf")) {
            std::cout << "SFML-CONSOLE: ERROR LOADING defaultFont << std::endl" << std::endl;
        }

        titleText.setString(titleStr);
        titleText.setFont(defaultFont);
        titleText.setCharacterSize(16);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(sf::Color::White);

        closeButton = titleText;
        closeButton.setString("x");

		//Load Cursors for resizing and input box
		arrowCursor.loadFromSystem(sf::Cursor::Arrow);
		hResizeCursor.loadFromSystem(sf::Cursor::SizeHorizontal);
		vResizeCursor.loadFromSystem(sf::Cursor::SizeVertical);
		diagResize1Cursor.loadFromSystem(sf::Cursor::SizeTopLeftBottomRight); // ↘↖
		diagResize2Cursor.loadFromSystem(sf::Cursor::SizeBottomLeftTopRight); // ↙↗	
 
        closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
        titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));
		// Default Logs that are printed upon creation, if you dont want these, set disableStartupLogs to true
		if (!disableStartupLogs) {
			logManager.addLog(defaultFont, "Console initialized", sf::Color::Green);
			logManager.addLog(defaultFont, "Welcome to sfml-console", sf::Color::Green);
			logManager.addLog(defaultFont, "For More information, type 'about' or go to https://github.com/clearlyyy/sfml-console and read the README.md", sf::Color::Cyan);
			logManager.addLog(defaultFont, "To disable these logs, set disableStartupLogs to true in SFMLConsole::createInstance()", sf::Color(235, 183, 42), 30);
			logManager.addLog(defaultFont, "To set the title of the console, try SFMLConsole::setTitle()", sf::Color::White, 14); 
			logManager.addLog(defaultFont, "Type 'help' to view available commands", sf::Color::Yellow);
		}
		
		// Pre-defined commands, if you dont want these, just remove them.
        cmdManager.addCommand("clear", std::bind(&ConsoleLogView::clear, &logManager)); 
        cmdManager.addCommand("help", std::bind(&SFMLConsole::displayCommands, this));

    }

	void handleResize(resizeType type) {
		std::cout << type << std::endl;
	}
	
	//Returns the region the user is trying to resize the window in.
	resizeType getResizeRegion(sf::RenderWindow& window) {
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePos);
		sf::Vector2f consolePos = bg.getPosition();

		float left = consolePos.x;
		float right = consolePos.x + consoleSize.x;
		float top = consolePos.y;
		float bottom = consolePos.y + consoleSize.y;

		float cornerSize = resizeZonePadding * 4.5f; // You can tweak this

		// Check corners first (bigger square regions)
		sf::FloatRect topLeftCorner(left - resizeZonePadding, top - resizeZonePadding, cornerSize, cornerSize);
		sf::FloatRect topRightCorner(right - cornerSize + resizeZonePadding, top - resizeZonePadding, cornerSize, cornerSize);
		sf::FloatRect bottomLeftCorner(left - resizeZonePadding, bottom - cornerSize + resizeZonePadding, cornerSize, cornerSize);
		sf::FloatRect bottomRightCorner(right - cornerSize + resizeZonePadding, bottom - cornerSize + resizeZonePadding, cornerSize, cornerSize);

		if (topLeftCorner.contains(mouseWorldPos))     return resizeType::TOPLEFT_CORNER;
		if (topRightCorner.contains(mouseWorldPos))    return resizeType::TOPRIGHT_CORNER;
		if (bottomLeftCorner.contains(mouseWorldPos))  return resizeType::BOTTOMLEFT_CORNER;
		if (bottomRightCorner.contains(mouseWorldPos)) return resizeType::BOTTOMRIGHT_CORNER;

		// Now check edges
		bool onLeftEdge = (mouseWorldPos.x >= left - resizeZonePadding && mouseWorldPos.x <= left + resizeZonePadding &&
						mouseWorldPos.y >= top && mouseWorldPos.y <= bottom);
		bool onRightEdge = (mouseWorldPos.x >= right - resizeZonePadding && mouseWorldPos.x <= right + resizeZonePadding &&
							mouseWorldPos.y >= top && mouseWorldPos.y <= bottom);
		bool onTopEdge = (mouseWorldPos.y >= top - resizeZonePadding && mouseWorldPos.y <= top + resizeZonePadding &&
						mouseWorldPos.x >= left && mouseWorldPos.x <= right);
		bool onBottomEdge = (mouseWorldPos.y >= bottom - resizeZonePadding && mouseWorldPos.y <= bottom + resizeZonePadding &&
							mouseWorldPos.x >= left && mouseWorldPos.x <= right);

		if (onLeftEdge)   return resizeType::LEFT_EDGE;
		if (onRightEdge)  return resizeType::RIGHT_EDGE;
		if (onTopEdge)    return resizeType::TOP_EDGE;
		if (onBottomEdge) return resizeType::BOTTOM_EDGE;

		return resizeType::NONE;
	}
	
    public:

	//Create the console object.
	static SFMLConsole& createInstance(sf::RenderWindow& window, bool disableStartupLogs = false, bool floating = true) {
		if (instance == nullptr) {
			instance = new SFMLConsole(window, disableStartupLogs);
		}
		return *instance;
	}
	//Retrieve the console object.
	static SFMLConsole& getInstance() {
		if (instance == nullptr) {
			throw std::runtime_error("SFMLConsole has not been initialized, use createInstance to create it.");
		}
		return *instance;
	}

	//Destroy the instance of the console. 
	static void destroyInstance() {
		delete instance;
		instance = nullptr;
	}
	
    // Update Function, place this inside your event loop, otherwise use a nullptr for event.
    void Update(sf::Event* event, sf::RenderWindow& window) {
		// Open/Close the console with openConsoleKey, default is tilde.
		if (sf::Keyboard::isKeyPressed(openConsoleKey)) {
			if (openConsoleClock.getElapsedTime().asSeconds() > 0.1 && !didConsoleJustOpen) {				
				isVisible = !isVisible;
				didConsoleJustOpen = true;
				openConsoleClock.restart();
			}
			
		} else {
			didConsoleJustOpen = false;
		}
		
		if (isVisible) {

			// Check if left mouse button is pressed
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
				if (!draggingConsole && titleBar.getGlobalBounds().contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
					// Start dragging and calculate the initial offset
					draggingConsole = true;
					offset = bg.getPosition() - sf::Vector2f(sf::Mouse::getPosition(window));
				}
				if (closeButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
					isVisible = false;
				}

			} else {
				// Stop dragging when left mouse is released
				draggingConsole = false;
			}

            if (floating) {
			    //Check if cursor is in a resize area, and update the cursor if so,
                if (!inputObj.isHoveringFlag() && !draggingConsole) {
			        resizeType region = getResizeRegion(window);
			        switch (region) {
			        	case resizeType::TOPLEFT_CORNER:
			        	case resizeType::BOTTOMRIGHT_CORNER:
			        		window.setMouseCursor(diagResize1Cursor);
			        		break;

			        	case resizeType::TOPRIGHT_CORNER:
			        	case resizeType::BOTTOMLEFT_CORNER:
			        		window.setMouseCursor(diagResize2Cursor);
			        		break;

			        	case resizeType::LEFT_EDGE:
			        	case resizeType::RIGHT_EDGE:
			        		window.setMouseCursor(hResizeCursor);
			        		break;

			        	case resizeType::TOP_EDGE:
			        	case resizeType::BOTTOM_EDGE:
			        		window.setMouseCursor(vResizeCursor);
			        		break;

			        	case resizeType::NONE:
			        		window.setMouseCursor(arrowCursor);
			        		break;
			        }
                }   
		
			    //Check if user is resizing the window
                if (!draggingConsole) {
			            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {  
			            	if (!resizingConsole) {
                                resizeType currentResizeZone = getResizeRegion(window);
                                if (currentResizeZone != resizeType::NONE) {
                                    resizingConsole = true;
                                    activeResizeRegion = currentResizeZone;
                                    resizeStartMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                                    resizeStartConsolePos = bg.getPosition();
                                    resizeStartConsoleSize = consoleSize;
                                }
                            }
			            }

                    if (resizingConsole) {
                        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                        sf::Vector2f delta = mousePos - resizeStartMousePos;

                        sf::Vector2f newSize = resizeStartConsoleSize;
                        sf::Vector2f newPos = resizeStartConsolePos;

                        switch (activeResizeRegion) {
                            case resizeType::RIGHT_EDGE:
                                newSize.x += delta.x;
                                break;
                            case resizeType::LEFT_EDGE:
                                newSize.x -= delta.x;
                                newPos.x += delta.x;
                                break;
                            case resizeType::BOTTOM_EDGE:
                                newSize.y += delta.y;
                                break;
                            case resizeType::TOP_EDGE:
                                newSize.y -= delta.y;
                                newPos.y += delta.y;
                                break;
                        
                            case resizeType::TOPLEFT_CORNER:
                                newSize.x -= delta.x;
                                newSize.y -= delta.y;
                                newPos.x += delta.x;
                                newPos.y += delta.y;
                                break;
                        
                            case resizeType::TOPRIGHT_CORNER:
                                newSize.x += delta.x;
                                newSize.y -= delta.y;
                                newPos.y += delta.y;
                                break;
                        
                            case resizeType::BOTTOMLEFT_CORNER:
                                newSize.x -= delta.x;
                                newSize.y += delta.y;
                                newPos.x += delta.x;
                                break;
                        
                            case resizeType::BOTTOMRIGHT_CORNER:
                                newSize.x += delta.x;
                                newSize.y += delta.y;
                                break;
                        
                            default: break;
                        }

                        const  float minWidth = 100.0f, minHeight = 100.f;
                        newSize.x = std::max(minWidth, newSize.x);
                        newSize.y = std::max(minHeight, newSize.y);

                        consoleSize = newSize;
                        bg.setSize(consoleSize);
                        bg.setPosition(newPos);

                        logManager.setSize(sf::Vector2f(consoleSize.x, consoleSize.y - inputHeight * 2));
                        logManager.setPosition(bg.getPosition());
                        logManager.handleResize(defaultFont);

                        titleBar.setSize(sf::Vector2f(consoleSize.x, titleBarHeight));
                        titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
                        inputObj.setSize(sf::Vector2f(
                            bg.getSize().x, //- 2 * inputPadding,
                            inputHeight //- 2 * inputPadding
                        ));
                        inputObj.setPosition(sf::Vector2f(
                            bg.getPosition().x, //+ inputPadding,
                            bg.getPosition().y + consoleSize.y - inputHeight //+ inputPadding
                        ));

                        closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
                        titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));

                    }
                }

            
            
			    // Move titleBar if dragging
			    // Probably will simplify this as its pretty fucked.
			    if (draggingConsole && !resizingConsole) {
			    	bg.setPosition(sf::Vector2f(sf::Mouse::getPosition(window)) + offset);
                    position = sf::Vector2f(sf::Mouse::getPosition(window)) + offset;
			    	titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
			    	//input.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y + consoleSize.y));
			    	closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
			    	titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));
			    	logManager.setPosition(bg.getPosition());

			    	inputObj.setSize(sf::Vector2f(
			    		bg.getSize().x,
			    		inputHeight //- 2 * inputPadding
			    	));
			    	inputObj.setPosition(sf::Vector2f(
			    		bg.getPosition().x, //+ inputPadding,
			    		bg.getPosition().y + consoleSize.y - inputHeight //+ inputPadding
			    	));

			    }
            }



            //If not a floating window, only allow resizing from the bottom.
            if (!floating) {

                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {  
                    if (!resizingConsole) {
                        resizeType currentResizeZone = getResizeRegion(window);
                        if (currentResizeZone != resizeType::NONE) {
                            resizingConsole = true;
                            activeResizeRegion = currentResizeZone;
                            resizeStartMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                            resizeStartConsolePos = bg.getPosition();
                            resizeStartConsoleSize = consoleSize;
                        }
                    }
                }

                if (!inputObj.isHoveringFlag() && !draggingConsole) {
			        resizeType region = getResizeRegion(window);
			        switch (region) {
			        	case resizeType::BOTTOM_EDGE:
			        		window.setMouseCursor(vResizeCursor);
			        		break;
			        	case resizeType::NONE:
			        		window.setMouseCursor(arrowCursor);
			        		break;

                        default: break;
			        }
                } 

                if (resizingConsole) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    sf::Vector2f delta = mousePos - resizeStartMousePos;
                    sf::Vector2f newSize = resizeStartConsoleSize;
                    sf::Vector2f newPos = resizeStartConsolePos;
                    switch (activeResizeRegion) {
                        case resizeType::BOTTOM_EDGE:
                            newSize.y += delta.y;
                            break;
                        default:
                            break;
                    }

                    consoleSize = newSize;
                    bg.setSize(consoleSize);
                    bg.setPosition(newPos);
                    logManager.setSize(sf::Vector2f(consoleSize.x, consoleSize.y - inputHeight * 2));
                    logManager.setPosition(bg.getPosition());
                    logManager.handleResize(defaultFont);
                    titleBar.setSize(sf::Vector2f(consoleSize.x, titleBarHeight));
                    titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
                    inputObj.setSize(sf::Vector2f(
                        bg.getSize().x, //- 2 * inputPadding,
                        inputHeight //- 2 * inputPadding
                    ));
                    inputObj.setPosition(sf::Vector2f(
                        bg.getPosition().x, //+ inputPadding,
                        bg.getPosition().y + consoleSize.y - inputHeight //+ inputPadding
                    ));
                }
            }



			if (event) {
				inputObj.handleEvent(*event, window);

				if (event->type == sf::Event::TextEntered) {
					m_textInputActive = true;
				}

                if (event->type == sf::Event::MouseButtonReleased && event->mouseButton.button == sf::Mouse::Left) {
                    resizingConsole = false;
                    activeResizeRegion = resizeType::NONE;
                }

				if (event->type == sf::Event::KeyPressed && event->key.code == sf::Keyboard::Enter) {
					std::string enteredText = inputObj.getText();
					inputObj.addToPastInputs(enteredText);
					inputObj.setCurrentPastInputsIndex(0);
					
					inputObj.clearText();
   					// Split entered text into command and arguments
   					std::istringstream stream(enteredText);
                    std::string command;
                    stream >> command; // First word is the command

					logManager.addLog(defaultFont, command, sf::Color::White);
                    std::vector<std::string> args;
                    std::string arg;

					while (stream >> arg) { // The rest are arguments
						args.push_back(arg);
					}
                                          
                    //Try to run the command.
                    executeCommand(command, args);                                        
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

    }

    /// @brief Forces a resize on the console window.
    void forceResize() {
        bg.setSize(consoleSize);
        bg.setPosition(position);
        logManager.setSize(sf::Vector2f(consoleSize.x, consoleSize.y - inputHeight * 2));
        logManager.setPosition(bg.getPosition());
        logManager.handleResize(defaultFont);
        titleBar.setSize(sf::Vector2f(consoleSize.x, titleBarHeight));
        titleBar.setPosition(sf::Vector2f(bg.getPosition().x, bg.getPosition().y  ));
        inputObj.setSize(sf::Vector2f(
            bg.getSize().x, //- 2 * inputPadding,
            inputHeight //- 2 * inputPadding
        ));
        inputObj.setPosition(sf::Vector2f(
            bg.getPosition().x, //+ inputPadding,
            bg.getPosition().y + consoleSize.y - inputHeight //+ inputPadding
        ));
        closeButton.setPosition(sf::Vector2f(titleBar.getPosition().x + titleBar.getSize().x - 30, titleBar.getPosition().y + (titleBar.getSize().y/2) - (closeButton.getLocalBounds().height / 2.f) - closeButton.getLocalBounds().top));
        titleText.setPosition(sf::Vector2f(titleBar.getPosition().x + 10.0f, titleBar.getPosition().y + (titleBar.getSize().y/2) - (titleText.getLocalBounds().height / 2.f) - titleText.getLocalBounds().top));
    }


    // Draw the Console, place this at the very top of your scene, otherwise your game may be drawn over it.
    void Draw(sf::RenderWindow& window) {
      if (isVisible) {
        window.draw(bg);
        logManager.DrawConsoleLog(window, defaultFont, bg, inputHeight, titleBarHeight);
        window.draw(titleBar);
        window.draw(titleText);
        window.draw(closeButton);
        inputObj.Draw(window);
      }
    }

    bool isConsoleVisible() {
        return isVisible;
    }

    void setVisiblity(bool isConsoleVisible) {
      isVisible = isConsoleVisible;
    }
  
    // Public CMD Functions
    void addCommand(std::string cmd, std::function<void(std::vector<std::string>)> func) {
        cmdManager.addCommand(cmd, func);
    }

    void log(std::string log, sf::Color color, float charSize = 16) {
		logManager.addLog(defaultFont, log, color, charSize);
	}

    // Set the Title of the console window
	void setTitle(std::string title) {
		titleStr = title;
		titleText.setString(title);
	}

    // For Performance reasons, you may want to lower the maximum amount of logs in the console, by default it is 500.
    void setMaxLogs(size_t MAX_LOGS) {
        logManager.setMaxLogs(MAX_LOGS);
    }

    // Set the size of the console
    void setSize(sf::Vector2f size) {
        consoleSize = size;
        forceResize();
    }

    // Set the size of the consoles x value;
    void setSizeX(float sizeX) {
        consoleSize.x = sizeX;
        forceResize();
    }
    // Set the size of the consoles y value;
    void setSizeY(float sizeY) {
        consoleSize.y = sizeY;
        forceResize();
    }

    void setBackgroundTransparency(float alpha) {
        backgroundColor = sf::Color(backgroundColor.r, backgroundColor.g, backgroundColor.b, alpha);
        bg.setFillColor(backgroundColor);
    }

    void setBackgroundColor(sf::Color color) {
        backgroundColor = color;
        bg.setFillColor(backgroundColor);
    }

};

inline SFMLConsole* SFMLConsole::instance = nullptr;
