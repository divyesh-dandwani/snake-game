#include <iostream>
#include <deque>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <conio.h>
#include <fstream>
#include <sstream>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
using namespace std;

// ============ Sound System ============
class SoundManager
{
public:
    static void playSound(const string &event)
    {
        // Cross-platform beep using terminal bell
        if (event == "eat")
        {
            cout << "\a" << flush; // Single beep for food
        }
        else if (event == "powerup")
        {
            cout << "\a" << flush;
            Sleep(50);
            cout << "\a" << flush; // Double beep for powerup
        }
        else if (event == "gameover")
        {
            for (int i = 0; i < 3; ++i)
            {
                cout << "\a" << flush;
                Sleep(100);
            }
        }
        else if (event == "collision")
        {
            cout << "\a" << flush;
        }
    }
};

struct Point
{
    int x, y;

    Point(int x = 0, int y = 0) : x(x), y(y) {}

    bool operator==(const Point &other) const
    {
        return x == other.x && y == other.y;
    }
};

// ============ PowerUp System ============
enum PowerUpType
{
    SPEED_BOOST,   // Faster movement
    SLOW_DOWN,     // Slower movement
    SCORE_DOUBLE,  // Double points for limited time
    INVINCIBILITY, // Can't die from obstacles/walls temporarily
    SHRINK         // Remove last segment
};

class PowerUp
{
private:
    Point position;
    PowerUpType type;
    int duration; // Duration in game ticks
    bool active;
    int remainingTime; // Remaining active time

public:
    PowerUp() : position(0, 0), type(SPEED_BOOST), duration(0), active(false), remainingTime(0) {}

    void spawn(int gridWidth, int gridHeight, const deque<Point> &snakeBody,
               const vector<Point> &obstacles, const vector<Point> &foodPositions)
    {
        bool validPosition;
        do
        {
            validPosition = true;
            position.x = rand() % gridWidth;
            position.y = rand() % gridHeight;

            // Check snake collision
            for (const auto &segment : snakeBody)
            {
                if (position == segment)
                {
                    validPosition = false;
                    break;
                }
            }

            // Check obstacle collision
            if (validPosition)
            {
                for (const auto &obs : obstacles)
                {
                    if (position == obs)
                    {
                        validPosition = false;
                        break;
                    }
                }
            }

            // Check food collision
            if (validPosition)
            {
                for (const auto &food : foodPositions)
                {
                    if (position == food)
                    {
                        validPosition = false;
                        break;
                    }
                }
            }
        } while (!validPosition);

        // Randomly select powerup type
        type = static_cast<PowerUpType>(rand() % 5);
        duration = 100; // 100 ticks duration for time-based powerups
        active = true;
        remainingTime = 0;
    }

    Point getPosition() const { return position; }
    PowerUpType getType() const { return type; }
    bool isActive() const { return active; }
    void deactivate() { active = false; }
    int getRemainingTime() const { return remainingTime; }

    void startEffect() { remainingTime = duration; }
    bool updateEffect()
    {
        if (remainingTime > 0)
        {
            remainingTime--;
            return true;
        }
        return false;
    }

    char getSymbol() const
    {
        switch (type)
        {
        case SPEED_BOOST:
            return 'S';
        case SLOW_DOWN:
            return 'L';
        case SCORE_DOUBLE:
            return 'D';
        case INVINCIBILITY:
            return 'I';
        case SHRINK:
            return 'R';
        default:
            return 'P';
        }
    }

    string getName() const
    {
        switch (type)
        {
        case SPEED_BOOST:
            return "Speed Boost";
        case SLOW_DOWN:
            return "Slow Motion";
        case SCORE_DOUBLE:
            return "Double Score";
        case INVINCIBILITY:
            return "Invincibility";
        case SHRINK:
            return "Shrink";
        default:
            return "PowerUp";
        }
    }
};

// ============ Obstacle Class ============
class Obstacle
{
private:
    vector<Point> positions;

public:
    Obstacle() {}

    void generateObstacles(int gridWidth, int gridHeight, const Point &snakeStart)
    {
        positions.clear();
        int numObstacles = (gridWidth * gridHeight) / 50;

        for (int i = 0; i < numObstacles; ++i)
        {
            int x = rand() % gridWidth;
            int y = rand() % gridHeight;
            Point obs(x, y);

            if (abs(x - snakeStart.x) > 3 && abs(y - snakeStart.y) > 3)
            {
                positions.push_back(obs);
            }
        }
    }

    bool isObstacle(const Point &p) const
    {
        for (const auto &obs : positions)
        {
            if (obs == p)
                return true;
        }
        return false;
    }

    const vector<Point> &getPositions() const
    {
        return positions;
    }
};

// ============ Food Class (Multiple Foods Support) ============
class FoodManager
{
private:
    vector<Point> foodPositions;
    int maxFoods;

public:
    FoodManager(int max = 3) : maxFoods(max) {}

    void spawnFood(int gridWidth, int gridHeight, const deque<Point> &snakeBody,
                   const Obstacle &obstacles, const vector<PowerUp> &powerups)
    {
        if (foodPositions.size() >= maxFoods)
            return;

        bool validPosition;
        Point newFood;
        do
        {
            validPosition = true;
            newFood.x = rand() % gridWidth;
            newFood.y = rand() % gridHeight;

            // Check snake collision
            for (const auto &segment : snakeBody)
            {
                if (newFood == segment)
                {
                    validPosition = false;
                    break;
                }
            }

            // Check obstacle collision
            if (validPosition && obstacles.isObstacle(newFood))
            {
                validPosition = false;
            }

            // Check existing food collision
            if (validPosition)
            {
                for (const auto &food : foodPositions)
                {
                    if (newFood == food)
                    {
                        validPosition = false;
                        break;
                    }
                }
            }

            // Check powerup collision
            if (validPosition)
            {
                for (const auto &powerup : powerups)
                {
                    if (powerup.isActive() && newFood == powerup.getPosition())
                    {
                        validPosition = false;
                        break;
                    }
                }
            }
        } while (!validPosition);

        foodPositions.push_back(newFood);
    }

    void initializeFoods(int gridWidth, int gridHeight, const deque<Point> &snakeBody,
                         const Obstacle &obstacles, const vector<PowerUp> &powerups)
    {
        foodPositions.clear();
        for (int i = 0; i < maxFoods; ++i)
        {
            spawnFood(gridWidth, gridHeight, snakeBody, obstacles, powerups);
        }
    }

    bool checkAndRemoveFood(const Point &position)
    {
        for (auto it = foodPositions.begin(); it != foodPositions.end(); ++it)
        {
            if (*it == position)
            {
                foodPositions.erase(it);
                return true;
            }
        }
        return false;
    }

    const vector<Point> &getFoodPositions() const
    {
        return foodPositions;
    }

    bool isFoodAt(const Point &p) const
    {
        for (const auto &food : foodPositions)
        {
            if (food == p)
                return true;
        }
        return false;
    }
};

// ============ Snake Class ============
class Snake
{
private:
    deque<Point> body;
    Point direction;
    Point nextDirection;
    bool growing;

public:
    Snake(int startX, int startY) : direction(1, 0), nextDirection(1, 0), growing(false)
    {
        body.push_back(Point(startX, startY));
        body.push_back(Point(startX - 1, startY));
        body.push_back(Point(startX - 2, startY));
    }

    void setDirection(int dx, int dy)
    {
        if (dx != -direction.x || dy != -direction.y)
        {
            nextDirection.x = dx;
            nextDirection.y = dy;
        }
    }

    void move()
    {
        direction = nextDirection;

        Point newHead(body.front().x + direction.x, body.front().y + direction.y);
        body.push_front(newHead);

        if (!growing)
        {
            body.pop_back();
        }
        else
        {
            growing = false;
        }
    }

    void grow()
    {
        growing = true;
    }

    void shrink()
    {
        if (body.size() > 3) // Keep minimum length
        {
            body.pop_back();
        }
    }

    Point getHead() const
    {
        return body.front();
    }

    const deque<Point> &getBody() const
    {
        return body;
    }

    bool checkSelfCollision() const
    {
        Point head = body.front();
        for (size_t i = 1; i < body.size(); ++i)
        {
            if (head == body[i])
            {
                return true;
            }
        }
        return false;
    }
};

// ============ HighScore Manager ============
class HighScoreManager
{
private:
    const string filename = "snake_highscore.dat";
    int highScore;

public:
    HighScoreManager() : highScore(0)
    {
        loadHighScore();
    }

    void loadHighScore()
    {
        ifstream file(filename);
        if (file.is_open())
        {
            file >> highScore;
            file.close();
        }
        else
        {
            highScore = 0;
        }
    }

    void saveHighScore(int score)
    {
        if (score > highScore)
        {
            highScore = score;
            ofstream file(filename);
            if (file.is_open())
            {
                file << highScore;
                file.close();
            }
        }
    }

    int getHighScore() const
    {
        return highScore;
    }
};

// ============ Game Class ============
class Game
{
private:
    int WIDTH;
    int HEIGHT;
    Snake *snake;
    Snake *snake2;
    FoodManager foodManager;
    Obstacle obstacles;
    vector<PowerUp> powerups;
    int score;
    int score2;
    int baseSpeed;
    int currentSpeed;
    bool gameOver;
    DWORD oldOutMode;
    HighScoreManager highScoreManager;
    vector<string> screenBuffer;
    vector<string> previousBuffer;

    // Active powerup effects
    bool invincibilityActive;
    bool doubleScoreActive;
    int invincibilityTimer;
    int doubleScoreTimer;

    void setupTerminal()
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleMode(hOut, &oldOutMode);
        DWORD dwMode = oldOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void restoreTerminal()
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleMode(hOut, oldOutMode);
    }

    void getTerminalSize()
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        WIDTH = min(60, columns - 4);
        HEIGHT = min(30, rows - 10);

        if (WIDTH < 20)
            WIDTH = 20;
        if (HEIGHT < 15)
            HEIGHT = 15;
    }

    void clearScreen()
    {
        cout << "\033[2J\033[H";
    }

    void moveCursor(int row, int col)
    {
        cout << "\033[" << row << ";" << col << "H";
    }

    void hideCursor()
    {
        cout << "\033[?25l";
    }

    void showCursor()
    {
        cout << "\033[?25h";
    }

    void initializeBuffer()
    {
        screenBuffer.clear();
        previousBuffer.clear();

        // Top border
        string topBorder = "+";
        for (int i = 0; i < WIDTH; ++i)
            topBorder += "-";
        topBorder += "+";
        screenBuffer.push_back(topBorder);

        // Game area
        for (int y = 0; y < HEIGHT; ++y)
        {
            string line = "|";
            for (int x = 0; x < WIDTH; ++x)
            {
                line += " ";
            }
            line += "|";
            screenBuffer.push_back(line);
        }

        // Bottom border
        string bottomBorder = "+";
        for (int i = 0; i < WIDTH; ++i)
            bottomBorder += "-";
        bottomBorder += "+";
        screenBuffer.push_back(bottomBorder);

        // Status lines
        screenBuffer.push_back("");
        screenBuffer.push_back("");
        screenBuffer.push_back("Controls: W/A/S/D or Arrow Keys | Q to quit");

        previousBuffer = screenBuffer;
    }

    void updateBuffer()
    {
        // Reset game area to spaces
        for (int y = 0; y < HEIGHT; ++y)
        {
            for (int x = 0; x < WIDTH; ++x)
            {
                screenBuffer[y + 1][x + 1] = ' ';
            }
        }

        // Draw obstacles
        for (const auto &obs : obstacles.getPositions())
        {
            if (obs.x >= 0 && obs.x < WIDTH && obs.y >= 0 && obs.y < HEIGHT)
            {
                screenBuffer[obs.y + 1][obs.x + 1] = '#';
            }
        }

        // Draw multiple foods
        for (const auto &foodPos : foodManager.getFoodPositions())
        {
            if (foodPos.x >= 0 && foodPos.x < WIDTH && foodPos.y >= 0 && foodPos.y < HEIGHT)
            {
                screenBuffer[foodPos.y + 1][foodPos.x + 1] = '*';
            }
        }

        // Draw powerups
        for (const auto &powerup : powerups)
        {
            if (powerup.isActive())
            {
                Point pos = powerup.getPosition();
                if (pos.x >= 0 && pos.x < WIDTH && pos.y >= 0 && pos.y < HEIGHT)
                {
                    screenBuffer[pos.y + 1][pos.x + 1] = powerup.getSymbol();
                }
            }
        }

        // Draw snake
        const deque<Point> &body = snake->getBody();
        for (size_t i = 0; i < body.size(); ++i)
        {
            if (body[i].x >= 0 && body[i].x < WIDTH && body[i].y >= 0 && body[i].y < HEIGHT)
            {
                if (i == 0)
                    screenBuffer[body[i].y + 1][body[i].x + 1] = 'O';
                else
                    screenBuffer[body[i].y + 1][body[i].x + 1] = 'o';
            }
        }

        const deque<Point> &body2 = snake2->getBody();
        for (size_t i = 0; i < body2.size(); ++i)
        {
            if (body2[i].x >= 0 && body2[i].x < WIDTH && body2[i].y >= 0 && body2[i].y < HEIGHT)
            {
                if (i == 0)
                    screenBuffer[body2[i].y + 1][body2[i].x + 1] = 'O';
                else
                    screenBuffer[body2[i].y + 1][body2[i].x + 1] = 'o';
            }
        }

        // Update status lines
        stringstream ss1;
        ss1 << "P1 Score: " << score << " | P2 Score: " << score2 << " | High Score: " << highScoreManager.getHighScore();
        screenBuffer[HEIGHT + 2] = ss1.str();

        stringstream ss2;
        ss2 << "Active Effects: ";
        if (invincibilityActive)
            ss2 << "[INVINCIBLE:" << invincibilityTimer << "] ";
        if (doubleScoreActive)
            ss2 << "[DOUBLE SCORE:" << doubleScoreTimer << "] ";
        if (!invincibilityActive && !doubleScoreActive)
            ss2 << "None";
        screenBuffer[HEIGHT + 3] = ss2.str();
    }

    void draw()
    {
        updateBuffer();

        for (size_t i = 0; i < screenBuffer.size(); ++i)
        {
            if (screenBuffer[i] != previousBuffer[i])
            {
                moveCursor(i + 1, 1);
                cout << screenBuffer[i];
                cout.flush();
                previousBuffer[i] = screenBuffer[i];
            }
        }
    }

    void drawFullScreen()
    {
        clearScreen();
        for (const auto &line : screenBuffer)
        {
            cout << line << endl;
        }
        cout.flush();
    }

    char getInput()
    {
        if (_kbhit())
        {
            int c = _getch();
            if (c == 0 || c == 224)
            {
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

    void processInput(char input)
    {
        switch (input)
        {
        case '^': // Player 1 (Arrow Keys)
            snake->setDirection(0, -1);
            break;
        case 'v':
            snake->setDirection(0, 1);
            break;
        case '<':
            snake->setDirection(-1, 0);
            break;
        case '>':
            snake->setDirection(1, 0);
            break;
        case 'w': // Player 2 (WASD)
        case 'W':
            snake2->setDirection(0, -1);
            break;
        case 's':
        case 'S':
            snake2->setDirection(0, 1);
            break;
        case 'a':
        case 'A':
            snake2->setDirection(-1, 0);
            break;
        case 'd':
        case 'D':
            snake2->setDirection(1, 0);
            break;
        case 'q':
        case 'Q':
            gameOver = true;
            break;
        }
    }

    int checkCollision()
    {
        Point head1 = snake->getHead();
        Point head2 = snake2->getHead();

        bool p1Lost = false;
        bool p2Lost = false;

        // Player 1 collisions
        if (head1.x < 0 || head1.x >= WIDTH || head1.y < 0 || head1.y >= HEIGHT) p1Lost = true;
        if (snake->checkSelfCollision()) p1Lost = true;
        if (obstacles.isObstacle(head1)) p1Lost = true;

        // Player 2 collisions
        if (head2.x < 0 || head2.x >= WIDTH || head2.y < 0 || head2.y >= HEIGHT) p2Lost = true;
        if (snake2->checkSelfCollision()) p2Lost = true;
        if (obstacles.isObstacle(head2)) p2Lost = true;

        // Snake-to-Snake collisions
        const auto& body2 = snake2->getBody();
        for (const auto& segment : body2) {
            if (head1 == segment) p1Lost = true;
        }
        
        const auto& body1 = snake->getBody();
        for (const auto& segment : body1) {
            if (head2 == segment) p2Lost = true;
        }

        if (p1Lost && !invincibilityActive) SoundManager::playSound("collision");
        if (p2Lost && !invincibilityActive) SoundManager::playSound("collision");

        if (invincibilityActive) {
            p1Lost = false;
            p2Lost = false;
        }

        if (p1Lost && p2Lost) return 3;
        if (p1Lost) return 1;
        if (p2Lost) return 2;

        return 0;
    }

    void checkFood()
    {
        bool foodEaten = false;
        if (foodManager.checkAndRemoveFood(snake->getHead()))
        {
            snake->grow();
            int points = doubleScoreActive ? 2 : 1;
            score += points;
            foodEaten = true;
        }
        else if (foodManager.checkAndRemoveFood(snake2->getHead()))
        {
            snake2->grow();
            int points = doubleScoreActive ? 2 : 1;
            score2 += points;
            foodEaten = true;
        }

        if (foodEaten) {
            SoundManager::playSound("eat");
            // Spawn new food to maintain count
            foodManager.spawnFood(WIDTH, HEIGHT, snake->getBody(), obstacles, powerups);
        }
    }

    void checkPowerUp()
    {
        Point head1 = snake->getHead();
        Point head2 = snake2->getHead();
        for (auto &powerup : powerups)
        {
            if (powerup.isActive())
            {
                if (head1 == powerup.getPosition()) {
                    SoundManager::playSound("powerup");
                    applyPowerUp(powerup, snake);
                    powerup.deactivate();
                    break;
                } else if (head2 == powerup.getPosition()) {
                    SoundManager::playSound("powerup");
                    applyPowerUp(powerup, snake2);
                    powerup.deactivate();
                    break;
                }
            }
        }
    }

    void applyPowerUp(PowerUp &powerup, Snake* targetSnake)
    {
        switch (powerup.getType())
        {
        case SPEED_BOOST:
            currentSpeed = baseSpeed * 2; // Faster
            break;
        case SLOW_DOWN:
            currentSpeed = baseSpeed / 2; // Slower
            break;
        case SCORE_DOUBLE:
            doubleScoreActive = true;
            doubleScoreTimer = 100;
            break;
        case INVINCIBILITY:
            invincibilityActive = true;
            invincibilityTimer = 100;
            break;
        case SHRINK:
            targetSnake->shrink();
            break;
        }
    }

    void updatePowerUpEffects()
    {
        // Update invincibility
        if (invincibilityActive)
        {
            invincibilityTimer--;
            if (invincibilityTimer <= 0)
            {
                invincibilityActive = false;
            }
        }

        // Update double score
        if (doubleScoreActive)
        {
            doubleScoreTimer--;
            if (doubleScoreTimer <= 0)
            {
                doubleScoreActive = false;
            }
        }

        // Reset speed if no speed powerup active
        bool hasSpeedPowerup = false;
        for (const auto &pu : powerups)
        {
            if (pu.getRemainingTime() > 0)
            {
                hasSpeedPowerup = true;
                break;
            }
        }
        if (!hasSpeedPowerup)
        {
            currentSpeed = baseSpeed;
        }
    }

public:
    Game() : WIDTH(40), HEIGHT(25), snake(nullptr), snake2(nullptr), foodManager(3), score(0), score2(0),
             baseSpeed(120000), currentSpeed(120000), gameOver(false),
             invincibilityActive(false), doubleScoreActive(false),
             invincibilityTimer(0), doubleScoreTimer(0) {}

    ~Game()
    {
        if (snake)
            delete snake;
        if (snake2)
            delete snake2;
    }

    void run()
    {
        srand(time(0));
        setupTerminal();
        hideCursor();

        bool running = true;

        while (running)
        {
            getTerminalSize();

            if (snake)
                delete snake;
            if (snake2)
                delete snake2;
            snake = new Snake(3 * WIDTH / 4, HEIGHT / 2);
            snake2 = new Snake(WIDTH / 4, HEIGHT / 2);
            score = 0;
            score2 = 0;
            gameOver = false;
            currentSpeed = baseSpeed;
            invincibilityActive = false;
            doubleScoreActive = false;
            powerups.clear();

            obstacles.generateObstacles(WIDTH, HEIGHT, Point(WIDTH / 2, HEIGHT / 2));
            foodManager.initializeFoods(WIDTH, HEIGHT, snake->getBody(), obstacles, powerups);

            // Spawn initial powerups
            for (int i = 0; i < 2; ++i)
            {
                PowerUp pu;
                pu.spawn(WIDTH, HEIGHT, snake->getBody(), obstacles.getPositions(),
                         foodManager.getFoodPositions());
                powerups.push_back(pu);
            }

            // Show instructions
            clearScreen();
            cout << "=== SNAKE GAME - ENHANCED EDITION ===" << endl;
            cout << "Controls: W/A/S/D or Arrow Keys" << endl;
            cout << "Goal: Eat food (*) and collect powerups!" << endl;
            cout << "\nPowerups:" << endl;
            cout << "  S - Speed Boost (faster movement)" << endl;
            cout << "  L - Slow Motion (easier control)" << endl;
            cout << "  D - Double Score (2x points)" << endl;
            cout << "  I - Invincibility (can't die)" << endl;
            cout << "  R - Shrink (remove tail segment)" << endl;
            cout << "\nAvoid walls (#), obstacles, and yourself!" << endl;
            cout << "Play Area: " << WIDTH << "x" << HEIGHT << endl;
            cout << "Current High Score: " << highScoreManager.getHighScore() << endl;
            cout << "\nPress any key to start..." << endl;

            while (getInput() == 0)
            {
                Sleep(10);
            }

            initializeBuffer();
            drawFullScreen();

            int tickCounter = 0;
            int loser = 0;

            // Game loop
            while (!gameOver)
            {
                draw();

                char input = getInput();
                if (input != 0)
                {
                    processInput(input);
                }

                snake->move();
                snake2->move();

                loser = checkCollision();
                if (loser != 0)
                {
                    gameOver = true;
                    SoundManager::playSound("gameover");
                }

                checkFood();
                checkPowerUp();
                updatePowerUpEffects();

                // Spawn new powerup periodically
                tickCounter++;
                if (tickCounter % 150 == 0)
                {
                    PowerUp pu;
                    pu.spawn(WIDTH, HEIGHT, snake->getBody(), obstacles.getPositions(),
                             foodManager.getFoodPositions());
                    powerups.push_back(pu);

                    // Keep maximum 3 powerups
                    if (powerups.size() > 3)
                    {
                        powerups.erase(powerups.begin());
                    }
                }

                Sleep(currentSpeed / 1000);
            }

            highScoreManager.saveHighScore(score);

            // Game over screen
            clearScreen();
            cout << "=== GAME OVER ===" << endl;
            if (loser == 1) cout << "Player 1 Lost!" << endl;
            else if (loser == 2) cout << "Player 2 Lost!" << endl;
            else if (loser == 3) cout << "Both Players Lost!" << endl;
            cout << "P1 Final Score: " << score << " | P2 Final Score: " << score2 << endl;
            cout << "High Score: " << highScoreManager.getHighScore() << endl;
            if ((score == highScoreManager.getHighScore() || score2 == highScoreManager.getHighScore()) && max(score, score2) > 0)
            {
                cout << "🎉 NEW HIGH SCORE! 🎉" << endl;
            }
            cout << endl;
            cout << "Press R to restart or Q to quit: ";
            cout.flush();

            char choice = 0;
            while (choice != 'r' && choice != 'R' && choice != 'q' && choice != 'Q')
            {
                choice = getInput();
                Sleep(10);
            }

            if (choice == 'q' || choice == 'Q')
            {
                running = false;
            }
        }

        showCursor();
        restoreTerminal();
        clearScreen();
        cout << "Thanks for playing!" << endl;
        cout << "Final High Score: " << highScoreManager.getHighScore() << endl;
    }
};

int main()
{
    Game game;
    game.run();
    return 0;
}
