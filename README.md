# sfml-console
## Single Header, Easy to use console library for SFML.

sfml-console is a lightweight, bloat-free in-game console designed specifically for SFML applications. Inspired by the powerful consoles found in games like Counter-Strike and Half-Life, it provides developers with a simple and intuitive way to add real-time debugging and command execution to their projects—with minimal setup and maximum flexibility.

<img src="pic1.png" width="700">

### Easy to use in your project!
Simply just drag and drop SFMLConsole.hpp into your project, include it, and you have full access to the console.

### Usage

```c++
#include "SFMLConsole.hpp" // Include it in any file.

// Create the console instance and pass in your sf::RenderWindow
SFMLConsole& console = SFMLConsole::createInstance(window);

while (window.isOpen())
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        // Call Update in your main SFML event loop
        console.Update(&event, window);
    }

    window.clear();
    // Then just draw it.
    console.Draw(window):
    window.display();
}

```


