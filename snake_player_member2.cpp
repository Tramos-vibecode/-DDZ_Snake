// Snake: Overdrive — первый этап логики змейки.
// Движение и управление; столкновения и рост добавляются следующими этапами.
#include <SFML/Graphics.hpp>
#include <deque>

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

enum class Direction { Up, Down, Left, Right };

class SnakePlayer {
private:
    std::deque<GridPos> body;
    Direction dir = Direction::Right;
    Direction nextDir = Direction::Right;
    float moveTimer = 0.0f;
    float baseStepDelay = 0.14f;

    void step() {
        dir = nextDir;
        GridPos head = body.front();
        switch (dir) {
        case Direction::Up:    head.y -= 1; break;
        case Direction::Down:  head.y += 1; break;
        case Direction::Left:  head.x -= 1; break;
        case Direction::Right: head.x += 1; break;
        }
        // На этапе движения края соединены, чтобы змейка оставалась видимой.
        head.x = (head.x + GRID_COLS) % GRID_COLS;
        head.y = (head.y + GRID_ROWS) % GRID_ROWS;
        body.push_front(head);
        body.pop_back();
    }

public:
    SnakePlayer() { resetLevel(); }

    void resetLevel() {
        const int startX = GRID_COLS / 2;
        const int startY = GRID_ROWS / 2;
        body = { {startX, startY}, {startX - 1, startY}, {startX - 2, startY} };
        dir = Direction::Right;
        nextDir = Direction::Right;
        moveTimer = 0.0f;
    }

    void handleInput(const sf::Event& event) {
        if (event.type != sf::Event::KeyPressed) return;
        // Проверяем относительно последнего шага, чтобы несколько нажатий
        // между шагами не позволили развернуть змейку в собственное тело.
        switch (event.key.code) {
        case sf::Keyboard::W:
        case sf::Keyboard::Up:
            if (dir != Direction::Down) nextDir = Direction::Up;
            break;
        case sf::Keyboard::S:
        case sf::Keyboard::Down:
            if (dir != Direction::Up) nextDir = Direction::Down;
            break;
        case sf::Keyboard::A:
        case sf::Keyboard::Left:
            if (dir != Direction::Right) nextDir = Direction::Left;
            break;
        case sf::Keyboard::D:
        case sf::Keyboard::Right:
            if (dir != Direction::Left) nextDir = Direction::Right;
            break;
        case sf::Keyboard::R:
            resetLevel();
            break;
        default:
            break;
        }
    }

    void update(float dt) {
        moveTimer += dt;
        while (moveTimer >= baseStepDelay) {
            moveTimer -= baseStepDelay;
            step();
        }
    }

    const std::deque<GridPos>& getBody() const { return body; }
    int getLength() const { return static_cast<int>(body.size()); }

    void draw(sf::RenderWindow& window) const {
        for (std::size_t i = 0; i < body.size(); ++i) {
            sf::RectangleShape segment({CELL_SIZE - 2.0f, CELL_SIZE - 2.0f});
            segment.setPosition(GRID_OFFSET_X + body[i].x * CELL_SIZE + 1.0f,
                                GRID_OFFSET_Y + body[i].y * CELL_SIZE + 1.0f);
            segment.setFillColor(i == 0 ? sf::Color(16, 185, 129)
                                       : sf::Color(5, 150, 105));
            window.draw(segment);
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT),
                            "Snake: Overdrive - WASD / Arrows | R: reset | Esc: exit",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    SnakePlayer player;
    sf::Clock clock;

    while (window.isOpen()) {
        const float dt = clock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close();
            } else {
                player.handleInput(event);
            }
        }
        if (!window.isOpen()) break;
        player.update(dt);
        window.clear(sf::Color(18, 22, 31));
        // Временный фон для отдельной проверки прототипа игрока.
        sf::RectangleShape arena({static_cast<float>(GRID_COLS * CELL_SIZE),
                                  static_cast<float>(GRID_ROWS * CELL_SIZE)});
        arena.setPosition(GRID_OFFSET_X, GRID_OFFSET_Y);
        arena.setFillColor(sf::Color(24, 30, 42));
        arena.setOutlineThickness(2.0f);
        arena.setOutlineColor(sf::Color(74, 85, 104));
        window.draw(arena);
        player.draw(window);
        window.display();
    }
    return 0;
}
