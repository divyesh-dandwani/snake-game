# Ubiquitous Language

## Core Entities

| Term | Definition | Aliases to avoid | In code |
| ----------- | ------------------------------------------------------- | --------------------- | -------------------------- |
| **Snake** | The player-controlled entity that grows when eating fruit | Player | `Snake` — `game.cpp:344` |
| **Fruit** | Consumable item that increases the snake's length and score | Food | `FoodManager` — `game.cpp:235` ⚠ |
| **Obstacle** | Static blocks on the board that cause a game-over collision | Wall, block | `Obstacle` — `game.cpp:192` |
| **PowerUp** | Items that grant temporary effects like invincibility or speed | Bonus, buff | `PowerUp` — `game.cpp:69` |

## Game Concepts

| Term | Definition | Aliases to avoid | In code |
| ----------- | ------------------------------------------------------- | --------------------- | -------------------------- |
| **Board** | The 2D bounded playing area where the game takes place | Grid, screen, terminal | `gridWidth`, `gridHeight` — `game.cpp:81` ⚠ |
| **Score** | Points accumulated by the player during a session | Points | `score` — `game.cpp:480` |
| **Tick** | A single update cycle of the game loop determining speed | Frame, update | `duration` (ticks) — `game.cpp:74` |
| **Collision** | When the snake's head intersects with an obstacle, itself, or the board boundary | Hit, crash | `checkCollision()` — `game.cpp:741` |

## Relationships

- A **Snake** moves across the **Board** one step per **Tick**.
- Eating a **Fruit** increases the **Score** and the **Snake**'s length.
- A **Collision** ends the game unless a specific **PowerUp** (Invincibility) is active.

## Example dialogue

> **Dev:** "When a **Collision** happens, does the game end immediately?"
> **Domain expert:** "Yes, unless the **Snake** has the Invincibility **PowerUp** active. If invincible, it just passes through."
> **Dev:** "Does eating a **Fruit** always increase the **Score** by 1?"
> **Domain expert:** "Usually, but if the Double Score **PowerUp** is active, it increases by 2. Both events play a sound, though."

## Flagged ambiguities

- "Grid" and "Terminal" are used interchangeably in the codebase to refer to the **Board**. The domain term **Board** should be used for the logical playing area, while "Terminal" should only refer to the underlying rendering system.

## Code drift

Places where the code's name for a concept is not the canonical term. Reported, not renamed.

| Canonical term | Called in code | Location              | Note                                                          |
| -------------- | -------------- | --------------------- | ------------------------------------------------------------- |
| **Fruit**      | `FoodManager`  | `game.cpp:235`        | The code uses "Food" (e.g. `foodPositions`), but domain uses "Fruit". |
| **Board**      | `gridWidth` / `WIDTH` | `game.cpp:81` / `game.cpp:474` | Code refers to grid or width/height, domain calls the whole thing the Board. |
