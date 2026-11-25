// Snake: Overdrive — поле, препятствия, еда и базовая смена волн.
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>

constexpr unsigned int SCREEN_WIDTH = 900;
constexpr unsigned int SCREEN_HEIGHT = 650;
constexpr int CELL_SIZE = 30;
constexpr int GRID_OFFSET_X = 30;
constexpr int GRID_OFFSET_Y = 90;
constexpr int GRID_COLS = 28;
constexpr int GRID_ROWS = 18;

struct GridPos {
    int x = 0;
    int y = 0;
    bool operator==(const GridPos& other) const {
        return x == other.x && y == other.y;
    }
};

sf::String toUtf8(const std::string& value) {
    return sf::String::fromUtf8(value.begin(), value.end());
}

class WorldWaveManager {
private:
    int currentWave = 1;
    int foodRemainingInWave = 5;
    bool victory = false;
    std::optional<GridPos> foodPos;
    std::vector<GridPos> staticWalls;
    std::mt19937 random;

    bool contains(const std::vector<GridPos>& cells, GridPos cell) const {
        return std::find(cells.begin(), cells.end(), cell) != cells.end();
    }

public:
    explicit WorldWaveManager(unsigned int seed = std::random_device{}()) : random(seed) {
        startWave(1);
    }

    bool isInside(GridPos cell) const {
        return cell.x >= 0 && cell.x < GRID_COLS && cell.y >= 0 && cell.y < GRID_ROWS;
    }

    bool isOccupied(GridPos cell) const {
        return !isInside(cell) || contains(staticWalls, cell);
    }

    // Проверяем все клетки: даже на заполненном поле нет бесконечного поиска
    // или возврата заранее занятой клетки после нескольких случайных попыток.
    std::optional<GridPos> randomFreeCell(const std::vector<GridPos>& snakeBody) {
        std::vector<GridPos> freeCells;
        for (int y = 0; y < GRID_ROWS; ++y) {
            for (int x = 0; x < GRID_COLS; ++x) {
                GridPos cell{x, y};
                if (!isOccupied(cell) && !contains(snakeBody, cell)) {
                    freeCells.push_back(cell);
                }
            }
        }
        if (freeCells.empty()) return std::nullopt;
        std::uniform_int_distribution<std::size_t> pick(0, freeCells.size() - 1);
        return freeCells[pick(random)];
    }

    bool spawnFood(const std::vector<GridPos>& snakeBody = {}) {
        foodPos = randomFreeCell(snakeBody);
        return foodPos.has_value();
    }

    void startWave(int waveNum, const std::vector<GridPos>& snakeBody = {}) {
        currentWave = std::clamp(waveNum, 1, 10);
        foodRemainingInWave = 4 + currentWave;
        victory = false;
        foodPos.reset();
        staticWalls.clear();

        // Стены появляются без повторов и не занимают переданное тело змейки.
        const int wallCount = currentWave * 2;
        for (int i = 0; i < wallCount; ++i) {
            const auto cell = randomFreeCell(snakeBody);
            if (!cell) break;
            staticWalls.push_back(*cell);
        }
        spawnFood(snakeBody);
    }

    // Вызывается игровой логикой после поедания; в прототипе — клавишей Enter.
    void collectFood(const std::vector<GridPos>& snakeBody = {}) {
        if (victory || !foodPos) return;
        foodPos.reset();
        --foodRemainingInWave;
        if (foodRemainingInWave == 0) {
            if (currentWave == 10) {
                victory = true;
            } else {
                startWave(currentWave + 1, snakeBody);
            }
            return;
        }
        spawnFood(snakeBody);
    }

    int getCurrentWave() const { return currentWave; }
    int getFoodRemaining() const { return foodRemainingInWave; }
    bool isVictory() const { return victory; }
    const std::vector<GridPos>& getWalls() const { return staticWalls; }
    const std::optional<GridPos>& getFoodPos() const { return foodPos; }

    void draw(sf::RenderWindow& window) const {
        sf::RectangleShape arena({static_cast<float>(GRID_COLS * CELL_SIZE),
                                  static_cast<float>(GRID_ROWS * CELL_SIZE)});
        arena.setPosition(GRID_OFFSET_X, GRID_OFFSET_Y);
        arena.setFillColor(sf::Color(22, 28, 38));
        arena.setOutlineThickness(2.0f);
        arena.setOutlineColor(sf::Color(74, 85, 104));
        window.draw(arena);

        sf::VertexArray lines(sf::Lines);
        const sf::Color gridColor(30, 38, 52);
        for (int col = 1; col < GRID_COLS; ++col) {
            const float x = static_cast<float>(GRID_OFFSET_X + col * CELL_SIZE);
            lines.append(sf::Vertex({x, static_cast<float>(GRID_OFFSET_Y)}, gridColor));
            lines.append(sf::Vertex({x, static_cast<float>(GRID_OFFSET_Y + GRID_ROWS * CELL_SIZE)}, gridColor));
        }
        for (int row = 1; row < GRID_ROWS; ++row) {
            const float y = static_cast<float>(GRID_OFFSET_Y + row * CELL_SIZE);
            lines.append(sf::Vertex({static_cast<float>(GRID_OFFSET_X), y}, gridColor));
            lines.append(sf::Vertex({static_cast<float>(GRID_OFFSET_X + GRID_COLS * CELL_SIZE), y}, gridColor));
        }
        window.draw(lines);

        for (const auto& cell : staticWalls) {
            sf::RectangleShape wall({CELL_SIZE - 2.0f, CELL_SIZE - 2.0f});
            wall.setPosition(GRID_OFFSET_X + cell.x * CELL_SIZE + 1.0f,
                             GRID_OFFSET_Y + cell.y * CELL_SIZE + 1.0f);
            wall.setFillColor(sf::Color(71, 85, 105));
            window.draw(wall);
        }
        if (foodPos) {
            sf::CircleShape food(CELL_SIZE / 2.0f - 2.0f);
            food.setPosition(GRID_OFFSET_X + foodPos->x * CELL_SIZE + 2.0f,
                             GRID_OFFSET_Y + foodPos->y * CELL_SIZE + 2.0f);
            food.setFillColor(sf::Color(16, 185, 129));
            food.setOutlineThickness(2.0f);
            food.setOutlineColor(sf::Color::White);
            window.draw(food);
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT),
                            "Snake: Overdrive - World prototype",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Font font;
    bool fontLoaded = false;
    for (const std::string path : {"C:/Windows/Fonts/arial.ttf", "font.ttf",
         "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
         "/System/Library/Fonts/Supplemental/Arial.ttf"}) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }
    if (!fontLoaded) {
        std::cerr << "Font not found. Place font.ttf in the working directory.\n";
        return 1;
    }

    // Пример данных игрока для проверки, что стены и еда не займут его клетки.
    const std::vector<GridPos> snakeBody = {{14, 9}, {13, 9}, {12, 9}};
    WorldWaveManager world;
    world.startWave(1, snakeBody);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type != sf::Event::KeyPressed) continue;
            switch (event.key.code) {
            case sf::Keyboard::Escape: window.close(); break;
            case sf::Keyboard::Enter: world.collectFood(snakeBody); break;
            case sf::Keyboard::N: world.startWave(world.getCurrentWave() + 1, snakeBody); break;
            case sf::Keyboard::R: world.startWave(1, snakeBody); break;
            default: break;
            }
        }
        if (!window.isOpen()) break;
        window.clear(sf::Color(18, 22, 31));
        world.draw(window);

        for (const auto& cell : snakeBody) {
            sf::RectangleShape reserved({CELL_SIZE - 4.0f, CELL_SIZE - 4.0f});
            reserved.setPosition(GRID_OFFSET_X + cell.x * CELL_SIZE + 2.0f,
                                 GRID_OFFSET_Y + cell.y * CELL_SIZE + 2.0f);
            reserved.setFillColor(sf::Color(56, 189, 248));
            window.draw(reserved);
        }
        const std::string status = world.isVictory()
            ? "Демонстрация завершена! R — начать сначала"
            : "Волна: " + std::to_string(world.getCurrentWave()) + " / 10   |   Осталось еды: " +
              std::to_string(world.getFoodRemaining()) + "   |   Стен: " + std::to_string(world.getWalls().size());
        sf::Text title(toUtf8(status), font, 21);
        title.setPosition(30.0f, 12.0f);
        window.draw(title);
        sf::Text hint(toUtf8("Enter — собрать еду | N — следующая волна | R — сброс | Esc — выход"), font, 17);
        hint.setPosition(30.0f, 47.0f);
        hint.setFillColor(sf::Color(148, 163, 184));
        window.draw(hint);
        window.display();
    }
    return 0;
}
