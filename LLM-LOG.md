# LLM-LOG

## 1. Setup

| Field | Details |
| :--- | :--- |
| **Model(s)** | Gemini 3.1 Pro (High) |
| **Tool / harness** | Antigravity IDE Agent (Agentic Coding Assistant) |
| **IDE / editor** | Antigravity IDE |
| **Did you paste this assignment document into it?** | Yes. The exact requirements for "Part A" and the LLM logging format were pasted directly into the chat prompt. |

## 2. The Session

### General Information
- **Total number of prompts it took to get working code:** 3 prompts.
- **What code was given to it:** The agent was granted access to the entire workspace. It autonomously read `game.cpp` (1013 lines) using its file-viewing tools to understand the existing logic and apply changes across the whole file.

### Prompts and Replies

#### Prompt 1
> "The current C++ snake game relies on Unix-specific headers like `termios.h` and `unistd.h`, so it fails to compile on Windows. Please update the codebase to make it completely compatible with Windows, replacing platform-dependent terminal controls with Windows equivalents like `<conio.h>`."

#### Reply 1 (Code Produced)
*The agent successfully replaced the Unix headers and updated the `Game::getInput()` and `Game::setupTerminal()` methods for Windows:*
```cpp
#include <windows.h>
#include <conio.h>
// ...
    void setupTerminal() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleMode(hOut, &oldOutMode);
        DWORD dwMode = oldOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    char getInput() {
        if (_kbhit()) {
            int c = _getch();
            if (c == 0 || c == 224) {
                int ext = _getch();
                if (ext == 72) return 'U'; // Up
                // ...
            }
            return (char)c;
        }
        return 0;
    }
```

#### Prompt 2
> "Please update the codebase to implement Part A of the assignment. 
> Implement a second snake into the game that is controlled using the W, A, S, D keys, while Player 1 retains their original controls. Both snakes should compete for the same food on the grid, and whoever eats it first grows. Both players should have their own separate scores visible during gameplay. The game should end if either snake collides with a wall, itself, or the other snake. Finally, announce which player lost on the Game Over screen.
> 
> Note: Do not add any extra features like new menus, colours, or rename any variables. Do not refactor or tidy up the existing code. Implement exactly this feature and nothing more."

#### Reply 2 (Code Produced)
*The agent added the second snake, scores, and updated the control loop, but mapped Player 1's down arrow to 'D', causing a compilation conflict with Player 2's WASD 'D'.*
```cpp
    Snake *snake;
    Snake *snake2;
    int score;
    int score2;
// ...
    void processInput(char input) {
        switch (input) {
        case 'U': // Player 1
            snake->setDirection(0, -1);
            break;
        case 'D': // ERROR: Conflicts with WASD
            snake->setDirection(0, 1);
            break;
        // ...
        case 'd': // Player 2 (WASD)
        case 'D':
            snake2->setDirection(1, 0);
            break;
        }
    }
```

#### Prompt 3
> "The code failed to compile because of a `duplicate case value` error in the switch statement inside `processInput`. You mapped both Player 1's Down arrow and Player 2's Right key to 'D'. Please fix this conflict so the game compiles and works."

#### Reply 3 (Code Produced)
*The agent successfully resolved the conflict by using distinct symbols for Player 1's arrow keys.*
```cpp
    char getInput() {
        if (_kbhit()) {
            int c = _getch();
            if (c == 0 || c == 224) {
                int ext = _getch();
                if (ext == 72) return '^'; // Up
                if (ext == 80) return 'v'; // Down
                if (ext == 77) return '>'; // Right
                if (ext == 75) return '<'; // Left
            }
            return (char)c;
        }
        return 0;
    }
```

#### Which attempt finally worked, and what was wrong with earlier ones?
The **third prompt** finally yielded fully working code. 
- **Prompt 1** successfully ported the game to Windows but did not add the multiplayer feature.
- **Prompt 2** implemented the multiplayer logic but resulted in a `duplicate case value` compilation error because the agent mapped both Player 1's "Arrow Down" and Player 2's "Right" (WASD) to the character `'D'`.
- **Prompt 3** explicitly asked the agent to resolve this collision by changing the arrow key mapping, which resulted in a successful compilation and a fully functioning local multiplayer game.
