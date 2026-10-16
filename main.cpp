#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <optional>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

// Constants
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int TILE_SIZE = 30;
const int OFFSET_X = 50;
const int OFFSET_Y = 50;

enum class Color {
    I = 0, O, T, S, Z, J, L, None
};

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

struct Tetromino {
    std::vector<Point> shape;
    Color color;
    Point pos;
};

const std::vector<std::vector<Point>> SHAPES[7] = {
    {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }}, // O (Yellow)
    {{ {0, 0}, {0, 1}, {0, 2}, {1, 2} }}, // J (Blue)
    {{ {1, 0}, {1, 1}, {1, 2}, {0, 2} }}, // L (Orange/Magenta)
    {{ {0, 1}, {1, 1}, {1, 0}, {2, 0} }}, // S (Green)
    {{ {0, 0}, {1, 0}, {1, 1}, {2, 1} }}, // Z (Red)
    {{ {0, 1}, {1, 1}, {2, 1}, {1, 0} }}, // T (Purple/Magenta)
    {{ {0, 0}, {0, 1}, {0, 2}, {0, 3} }}  // I (Cyan)
};

sf::Color getColor(Color c) {
    switch (c) {
        case Color::I: return sf::Color::Cyan;
        case Color::O: return sf::Color::Yellow;
        case Color::T: return sf::Color::Magenta;
        case Color::S: return sf::Color::Green;
        case Color::Z: return sf::Color::Red;
        case Color::J: return sf::Color::Blue;
        case Color::L: return sf::Color(255, 165, 0); // Orange
        default: return sf::Color::Black;
    }
}

class Tetris {
    int board[BOARD_WIDTH][BOARD_HEIGHT] = {0};
    Tetromino currentPiece;
    Tetromino nextPiece;
    int score = 0;
    int highScore = 0;
    int lines = 0;
    int level = 1;
    bool paused = false;
    bool gameOver = false;
    float dropTimer = 0;

    sf::SoundBuffer moveBuffer, rotateBuffer, dropBuffer, lockBuffer, clearBuffer, overBuffer;
    std::vector<std::unique_ptr<sf::Sound>> soundPool;
    const size_t MAX_SOUNDS = 8;

    void playSound(sf::SoundBuffer& buffer) {
        if (buffer.getSampleCount() == 0) return;
        for (auto& s : soundPool) {
            if (s && s->getStatus() == sf::Sound::Status::Stopped) {
                s->setBuffer(buffer);
                s->play();
                return;
            }
        }
    }

    std::string getSavePath() {
        try {
            const char* appData = std::getenv("APPDATA");
            if (appData) {
                fs::path path = fs::path(appData) / "TetrisGame";
                fs::create_directories(path);
                return (path / "highscore.txt").string();
            }
        } catch (...) {}
        return "highscore.txt";
    }

    void loadHighScore() {
        std::string path = getSavePath();
        std::ifstream file(path);
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }
    }

    void saveHighScore() {
        std::ofstream file(getSavePath());
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }

    void spawnPiece() {
        int id = rand() % 7;
        currentPiece = { SHAPES[id][0], static_cast<Color>(id), {BOARD_WIDTH / 2 - 1, 0} };
        if (checkCollision(currentPiece.pos, currentPiece.shape)) {
            gameOver = true;
            playSound(overBuffer);
        }
    }

    bool checkCollision(Point pos, const std::vector<Point>& shape) {
        for (auto& p : shape) {
            int nx = pos.x + p.x;
            int ny = pos.y + p.y;
            if (nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT) return true;
            if (ny >= 0 && board[nx][ny]) return true;
        }
        return false;
    }

    void rotate() {
        std::vector<Point> rotated;
        for (auto& p : currentPiece.shape) {
            rotated.push_back({ -p.y, p.x });
        }
        int minX = rotated[0].x, minY = rotated[0].y;
        for (auto& p : rotated) {
            if (p.x < minX) minX = p.x;
            if (p.y < minY) minY = p.y;
        }
        for (auto& p : rotated) { p.x -= minX; p.y -= minY; }

        if (!checkCollision(currentPiece.pos, rotated)) {
            currentPiece.shape = rotated;
            playSound(rotateBuffer);
        }
    }

    void clearLines() {
        int cleared = 0;
        for (int y = BOARD_HEIGHT - 1; y >= 0; --y) {
            bool full = true;
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                if (!board[x][y]) { full = false; break; }
            }
            if (full) {
                cleared++;
                for (int ty = y; ty > 0; --ty) {
                    for (int tx = 0; tx < BOARD_WIDTH; ++tx) board[tx][ty] = board[tx][ty - 1];
                }
                for (int tx = 0; tx < BOARD_WIDTH; ++tx) board[tx][0] = 0;
                y++;
            }
        }
        if (cleared > 0) {
            lines += cleared;
            score += cleared * 100 * level;
            level = 1 + lines / 10;
            playSound(clearBuffer);
            if (score > highScore) {
                highScore = score;
                saveHighScore();
            }
        }
    }

    void lockPiece() {
        for (auto& p : currentPiece.shape) {
            int nx = currentPiece.pos.x + p.x;
            int ny = currentPiece.pos.y + p.y;
            if (ny >= 0) board[nx][ny] = 1;
        }
        playSound(lockBuffer);
        clearLines();
        spawnPiece();
    }

public:
    Tetris() {
        srand(static_cast<unsigned int>(time(NULL)));
        
        // Generate buffers first
        generateSound(moveBuffer, 440.f, 0.05f);
        generateSound(rotateBuffer, 554.37f, 0.05f);
        generateSound(dropBuffer, 220.f, 0.1f);
        generateSound(lockBuffer, 110.f, 0.1f);
        generateSound(clearBuffer, 880.f, 0.2f);
        generateSound(overBuffer, 110.f, 0.5f);

        // Initialize sounds with a valid buffer (SFML 3.0.2 requires a buffer)
        for (size_t i = 0; i < MAX_SOUNDS; ++i) {
            soundPool.push_back(std::make_unique<sf::Sound>(moveBuffer));
        }
        
        loadHighScore();
        spawnPiece();
    }


    void generateSound(sf::SoundBuffer& buffer, float freq, float duration, float volume = 100.f) {
        const int sampleRate = 44100;
        std::vector<int16_t> samples;
        size_t totalSamples = static_cast<size_t>(sampleRate * duration);
        for (size_t i = 0; i < totalSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float amplitude = 32767.f * (volume / 100.f) * std::exp(-5.f * t);
            samples.push_back(static_cast<int16_t>(amplitude * std::sin(2.f * 3.14159f * freq * t)));
        }
        std::vector<sf::SoundChannel> channelMap = { sf::SoundChannel::Mono };
        buffer.loadFromSamples(samples.data(), samples.size(), 1, 44100, channelMap);
    }

    void update(float dt) {
        if (paused || gameOver) return;
        dropTimer += dt;
        float dropInterval = 1.0f / (1.0f + level * 0.5f);
        if (dropTimer >= dropInterval) {
            Point nextPos = { currentPiece.pos.x, currentPiece.pos.y + 1 };
            if (!checkCollision(nextPos, currentPiece.shape)) {
                currentPiece.pos = nextPos;
            } else {
                lockPiece();
            }
            dropTimer = 0;
        }
    }

    void move(int dx, int dy) {
        if (paused || gameOver) return;
        Point nextPos = { currentPiece.pos.x + dx, currentPiece.pos.y + dy };
        if (!checkCollision(nextPos, currentPiece.shape)) {
            currentPiece.pos = nextPos;
            if (dx != 0) playSound(moveBuffer);
        } else if (dy > 0) {
            lockPiece();
        }
    }

    void hardDrop() {
        if (paused || gameOver) return;
        while (!checkCollision({ currentPiece.pos.x, currentPiece.pos.y + 1 }, currentPiece.shape)) {
            currentPiece.pos.y++;
        }
        playSound(dropBuffer);
        lockPiece();
    }

    void rotatePiece() { rotate(); }
    void togglePause() { paused = !paused; }
    void restart() {
        std::memset(board, 0, sizeof(board));
        score = 0; lines = 0; level = 1;
        gameOver = false; paused = false;
        spawnPiece();
    }

    void draw(sf::RenderWindow& window) {
        // 1. Draw Board and Locked Pieces
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            for (int y = 0; y < BOARD_HEIGHT; ++y) {
                sf::RectangleShape rect({ (float)TILE_SIZE - 2, (float)TILE_SIZE - 2 });
                rect.setPosition(sf::Vector2f(OFFSET_X + x * TILE_SIZE, OFFSET_Y + y * TILE_SIZE));
                rect.setFillColor(board[x][y] ? sf::Color(100, 100, 100) : sf::Color(30, 30, 30));
                window.draw(rect);
            }
        }

        // Danger Line
        sf::RectangleShape dangerLine({ (float)BOARD_WIDTH * TILE_SIZE, 2.f });
        dangerLine.setPosition(sf::Vector2f(OFFSET_X, OFFSET_Y + 2 * TILE_SIZE));
        dangerLine.setFillColor(sf::Color::Red);
        window.draw(dangerLine);
 
        // 2. Ghost Piece
        Point ghostPos = currentPiece.pos;
        while (!checkCollision(ghostPos, currentPiece.shape)) {
            ghostPos.y++;
        }
        ghostPos.y--;


        for (auto& p : currentPiece.shape) {
            sf::RectangleShape rect({ (float)TILE_SIZE - 2, (float)TILE_SIZE - 2 });
            rect.setPosition(sf::Vector2f(OFFSET_X + (ghostPos.x + p.x) * TILE_SIZE, OFFSET_Y + (ghostPos.y + p.y) * TILE_SIZE));
            sf::Color gc = getColor(currentPiece.color);
            gc.a = 80;
            rect.setFillColor(gc);
            window.draw(rect);
        }

        // 3. Current Piece
        for (auto& p : currentPiece.shape) {
            sf::RectangleShape rect({ (float)TILE_SIZE - 2, (float)TILE_SIZE - 2});
            rect.setPosition(sf::Vector2f(OFFSET_X + (currentPiece.pos.x + p.x) * TILE_SIZE, OFFSET_Y + (currentPiece.pos.y + p.y) * TILE_SIZE));
            rect.setFillColor(getColor(currentPiece.color));
            window.draw(rect);
        }
    }

    int getScore() const { return score; }
    int getHighScore() const { return highScore; }
    int getLevel() const { return level; }
    int getLines() const { return lines; }
    bool isPaused() const { return paused; }
    bool isGameOver() const { return gameOver; }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ 600, 700 }), "Tetris SFML 3");
    Tetris game;
    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Font not found!" << std::endl;
    }

    sf::Text text(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                auto key = keyPressed->code;
                if (key == sf::Keyboard::Key::A || key == sf::Keyboard::Key::Left) game.move(-1, 0);
                if (key == sf::Keyboard::Key::D || key == sf::Keyboard::Key::Right) game.move(1, 0);
                if (key == sf::Keyboard::Key::S || key == sf::Keyboard::Key::Down) game.move(0, 1);
                if (key == sf::Keyboard::Key::W || key == sf::Keyboard::Key::Up) game.rotatePiece();
                if (key == sf::Keyboard::Key::Space) game.hardDrop();
                if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape) game.togglePause();
                if (key == sf::Keyboard::Key::R) game.restart();
            }
        }

        game.update(dt);

        window.clear(sf::Color(20, 20, 20));
        game.draw(window);

        text.setString("Score: " + std::to_string(game.getScore()) + 
                      "\nHigh Score: " + std::to_string(game.getHighScore()) +
                      "\nLevel: " + std::to_string(game.getLevel()) + 
                      "\nLines: " + std::to_string(game.getLines()) + 
                      "\n\nControls:\nWASD/Arrows: Move/Rotate\nSpace: Hard Drop\nP: Pause\nR: Restart");
        text.setPosition(sf::Vector2f(400, 50));
        window.draw(text);

        if (game.isPaused()) {
            sf::Text pText(font); pText.setCharacterSize(30); pText.setFillColor(sf::Color::Yellow);
            pText.setString("PAUSED"); pText.setPosition(sf::Vector2f(220, 300));
            window.draw(pText);
        }

        if (game.isGameOver()) {
            sf::Text goText(font); goText.setCharacterSize(40); goText.setFillColor(sf::Color::Red);
            goText.setString("GAME OVER\nPress R to Restart");
            goText.setPosition(sf::Vector2f(180, 300));
            window.draw(goText);
        }

        window.display();
    }
    return 0;
}
