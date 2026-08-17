# Code Smell Audit (main)

| Confidence | Smell | File:Line | Description | Severity |
|------------|-------|-----------|-------------|----------|
| C4 | Large Class | `game.cpp:471` | `Game` handles rendering, input, game logic, powerups, and collision. | HIGH |
| C3 | Data Clumps | `game.cpp:81` | `gridWidth` and `gridHeight` are passed together to `spawn()` methods across multiple classes instead of a `Board` object. | MEDIUM |
| C3 | Long Parameter List | `game.cpp:81` | `PowerUp::spawn` takes 5 parameters, making it hard to call. | MEDIUM |
| C3 | Switch Statements | `game.cpp:152` | `PowerUp::getSymbol` and `getName` (line 171) switch over `PowerUpType`, indicating missing polymorphism. | MEDIUM |
| C2 | Feature Envy | `game.cpp:741` | `checkCollision` heavily queries `snake` and `obstacles` internals to determine logic. | LOW |
| C2 | Temporary Field | `game.cpp:350` | `growing` field in `Snake` is only briefly true after eating, could just be local logic. | LOW |
