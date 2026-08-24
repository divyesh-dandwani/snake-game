# Code Smell Audit (feat/multiplayer - Lab 1)

| Confidence | Smell | File:Line | Description | Severity |
|------------|-------|-----------|-------------|----------|
| C4 | Large Class | `game.cpp:520` | `Game` has grown even larger with the addition of `snake2` and `score2`, handling logic for both players inside the main loop. | HIGH |
| C4 | Duplicate Code | `game.cpp:780` | `checkFood` contains duplicated blocks for `snake` and `snake2` to check for food consumption and increment scores. | HIGH |
| C4 | Duplicate Code | `game.cpp:802` | `checkPowerUp` contains nearly identical logic paths for `head1` and `head2`. | HIGH |
| C4 | Data Clumps | `game.cpp:525` | Player state variables (`snake`, `score`, `snake2`, `score2`) are passed and managed separately instead of being grouped into a `Player` or `SnakeState` object. | HIGH |
| C3 | Long Method | `game.cpp:610` | `Game::run()` handles setup, loop logic, game over, and both snakes, making it very difficult to read. | MEDIUM |
| C3 | Switch Statements | `game.cpp:823` | `applyPowerUp` still uses a switch statement instead of polymorphism, which makes adding new powerups harder. | MEDIUM |
