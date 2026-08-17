#include <iostream>
#include <deque>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <fstream>
#include <sys/ioctl.h>
#include <sstream>
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
            usleep(50000);
            cout << "\a" << flush; // Double beep for powerup
        }
        else if (event == "gameover")
        {
            for (int i = 0; i < 3; ++i)
            {
                cout << "\a" << flush;
                usleep(100000);
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
    struct PlayerState {
        Snake* snake;
        int score;
    };
    vector<PlayerState> players;
    FoodManager foodManager;
    Obstacle obstacles;
    vector<PowerUp> powerups;
    int baseSpeed;
    int currentSpeed;
    bool gameOver;
    struct termios oldt, newt;
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
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    void restoreTerminal()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }

    void getTerminalSize()
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        WIDTH = min(60, (int)w.ws_col - 4);
        HEIGHT = min(30, (int)w.ws_row - 10);

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

        // Draw snakes
        for (const auto& p : players) {
            const deque<Point> &body = p.snake->getBody();
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
        }

        // Update status lines
        stringstream ss1;
        ss1 << "Score: ";
        for (size_t i = 0; i < players.size(); ++i) {
            ss1 << "P" << (i+1) << ":" << players[i].score << " ";
        }
        ss1 << "| High Score: " << highScoreManager.getHighScore();
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
        char c = 0;
        read(STDIN_FILENO, &c, 1);

        if (c == 27)
        {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 1)
            {
                if (seq[0] == '[')
                {
                    if (read(STDIN_FILENO, &seq[1], 1) == 1)
                    {
                        if (seq[1] == 'A')
                            return '^';
                        if (seq[1] == 'B')
                            return 'v';
                        if (seq[1] == 'C')
                            return '>';
                        if (seq[1] == 'D')
                            return '<';
                    }
                }
            }
        }

        return c;
    }

    void processInput(char input)
    {
        if (players.empty()) return;
        switch (input)
        {
        // Player 1 (Arrow Keys)
        case '^':
            players[0].snake->setDirection(0, -1);
            break;
        case 'v':
            players[0].snake->setDirection(0, 1);
            break;
        case '<':
            players[0].snake->setDirection(-1, 0);
            break;
        case '>':
            players[0].snake->setDirection(1, 0);
            break;
            
        // Player 2 (WASD)
        case 'w':
        case 'W':
            if (players.size() > 1) players[1].snake->setDirection(0, -1);
            break;
        case 's':
        case 'S':
            if (players.size() > 1) players[1].snake->setDirection(0, 1);
            break;
        case 'a':
        case 'A':
            if (players.size() > 1) players[1].snake->setDirection(-1, 0);
            break;
        case 'd':
        case 'D':
            if (players.size() > 1) players[1].snake->setDirection(1, 0);
            break;
            
        case 'q':
        case 'Q':
            gameOver = true;
            break;
        }
    }

    bool checkCollision()
    {
        for (auto& p : players) {
            Point head = p.snake->getHead();

            // Wall collision (ignore if invincible)
            if (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT)
            {
                if (!invincibilityActive)
                {
                    SoundManager::playSound("collision");
                    return true;
                }
            }

            // Self collision (ignore if invincible)
            if (p.snake->checkSelfCollision())
            {
                if (!invincibilityActive)
                {
                    SoundManager::playSound("collision");
                    return true;
                }
            }

            // Obstacle collision (ignore if invincible)
            if (obstacles.isObstacle(head))
            {
                if (!invincibilityActive)
                {
                    SoundManager::playSound("collision");
                    return true;
                }
            }
        }
        return false;
    }

    void checkFood()
    {
        for (auto& p : players) {
            if (foodManager.checkAndRemoveFood(p.snake->getHead()))
            {
                p.snake->grow();
                int points = doubleScoreActive ? 2 : 1;
                p.score += points;
                SoundManager::playSound("eat");

                // Spawn new food to maintain count
                foodManager.spawnFood(WIDTH, HEIGHT, players[0].snake->getBody(), obstacles, powerups);
            }
        }
    }

    void checkPowerUp()
    {
        for (auto& p : players) {
            Point head = p.snake->getHead();
            for (auto &powerup : powerups)
            {
                if (powerup.isActive() && head == powerup.getPosition())
                {
                    SoundManager::playSound("powerup");
                    applyPowerUp(powerup, p.snake);
                    powerup.deactivate();
                }
            }
        }
    }

    void applyPowerUp(PowerUp &powerup, Snake* targetSnake)
    {
        switch (powerup.getType())
        {
        case SPEED_BOOST:
            currentSpeed = baseSpeed / 2;
            break;
        case SLOW_DOWN:
            currentSpeed = baseSpeed * 2;
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
    Game() : WIDTH(40), HEIGHT(25), foodManager(3),
             baseSpeed(120000), currentSpeed(120000), gameOver(false),
             invincibilityActive(false), doubleScoreActive(false),
             invincibilityTimer(0), doubleScoreTimer(0) {}

    ~Game()
    {
        for (auto& p : players) {
            delete p.snake;
        }
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

            for (auto& p : players) delete p.snake;
            players.clear();
            
            // Refactored: the number of snakes is controlled here
            int numPlayers = 2; 
            for (int i = 0; i < numPlayers; ++i) {
                // Different start positions for different players
                int startX = (i == 0) ? (3 * WIDTH / 4) : (WIDTH / 4);
                players.push_back({new Snake(startX, HEIGHT / 2), 0});
            }

            gameOver = false;
            currentSpeed = baseSpeed;
            invincibilityActive = false;
            doubleScoreActive = false;
            powerups.clear();

            obstacles.generateObstacles(WIDTH, HEIGHT, players[0].snake->getHead());
            foodManager.initializeFoods(WIDTH, HEIGHT, players[0].snake->getBody(), obstacles, powerups);

            // Spawn initial powerups
            for (int i = 0; i < 2; ++i)
            {
                PowerUp pu;
                pu.spawn(WIDTH, HEIGHT, players[0].snake->getBody(), obstacles.getPositions(),
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
                usleep(10000);
            }

            initializeBuffer();
            drawFullScreen();

            int tickCounter = 0;

            // Game loop
            while (!gameOver)
            {
                draw();

                char input = getInput();
                if (input != 0)
                {
                    processInput(input);
                }

                for (auto& p : players) {
                    p.snake->move();
                }

                if (checkCollision())
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
                    pu.spawn(WIDTH, HEIGHT, players[0].snake->getBody(), obstacles.getPositions(),
                             foodManager.getFoodPositions());
                    powerups.push_back(pu);

                    // Keep maximum 3 powerups
                    if (powerups.size() > 3)
                    {
                        powerups.erase(powerups.begin());
                    }
                }

                usleep(currentSpeed);
            }

            int maxScore = 0;
            for (const auto& p : players) {
                if (p.score > maxScore) maxScore = p.score;
            }
            highScoreManager.saveHighScore(maxScore);

            // Game over screen
            clearScreen();
            cout << "=== GAME OVER ===" << endl;
            for (size_t i = 0; i < players.size(); ++i) {
                cout << "P" << (i+1) << " Final Score: " << players[i].score << endl;
            }
            cout << "High Score: " << highScoreManager.getHighScore() << endl;
            if (maxScore == highScoreManager.getHighScore() && maxScore > 0)
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
                usleep(10000);
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
