// Snake: Overdrive — интерфейс: меню, игровые показатели и пауза.
#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

constexpr unsigned int SCREEN_WIDTH = 900;
constexpr unsigned int SCREEN_HEIGHT = 650;
constexpr float HUD_HEIGHT = 70.0f;

namespace Palette {
    const sf::Color Bg(18, 22, 31);
    const sf::Color HudBg(26, 32, 44);
    const sf::Color Panel(35, 43, 59);
    const sf::Color Text(240, 244, 248);
    const sf::Color TextMuted(148, 163, 184);
    const sf::Color Accent(16, 185, 129);
    const sf::Color AccentHover(5, 150, 105);
    const sf::Color Danger(239, 68, 68);
    const sf::Color Border(74, 85, 104);
}

sf::String toSfString(const std::string& utf8Str) {
    return sf::String::fromUtf8(utf8Str.begin(), utf8Str.end());
}

// Центрирование sf::Text внутри прямоугольника или по точке
void centerText(sf::Text& text, float x, float y) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    text.setPosition(x, y);
}

void centerTextX(sf::Text& text, float x, float topY) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top);
    text.setPosition(x, topY);
}

class Button {
public:
    sf::RectangleShape shape;
    std::string label;
    std::function<void()> onClick;
    sf::Color bgColor;
    sf::Color hoverColor;
    bool isHovered = false;

    Button(float x, float y, float w, float h, const std::string& text,
           std::function<void()> action,
           sf::Color bg = Palette::Panel,
           sf::Color hover = Palette::AccentHover)
        : label(text), onClick(action), bgColor(bg), hoverColor(hover)
    {
        shape.setPosition(x, y);
        shape.setSize({w, h});
        shape.setFillColor(bgColor);
        shape.setOutlineThickness(2.0f);
        shape.setOutlineColor(Palette::Border);
    }

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if (event.type == sf::Event::MouseMoved) {
            isHovered = shape.getGlobalBounds().contains(mousePos);
        } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (shape.getGlobalBounds().contains(mousePos) && onClick) {
                onClick();
                return true;
            }
        }
        return false;
    }

    void draw(sf::RenderWindow& window, const sf::Font& font) {
        shape.setFillColor(isHovered ? hoverColor : bgColor);
        shape.setOutlineColor(isHovered ? Palette::Accent : Palette::Border);
        window.draw(shape);

        sf::Text textObj(toSfString(label), font, 20);
        textObj.setFillColor(Palette::Text);
        sf::Vector2f pos = shape.getPosition();
        sf::Vector2f size = shape.getSize();
        centerText(textObj, pos.x + size.x / 2.0f, pos.y + size.y / 2.0f);
        window.draw(textObj);
    }
};

enum class UIState { MainMenu, Controls, About, Playing, Pause };

// Данные для подключения логики игрока и игрового мира.
struct GameStats {
    int score = 0;
    int length = 3;
    int lives = 3;
    int maxLives = 3;
    int level = 1;
    int foodLeft = 10;
    int eatenTotal = 0;

    void reset() { *this = GameStats{}; }
};

class HUD {
public:
    void draw(sf::RenderWindow& window, const sf::Font& font, const GameStats& stats) const {
        sf::RectangleShape background({static_cast<float>(SCREEN_WIDTH), HUD_HEIGHT});
        background.setFillColor(Palette::HudBg);
        window.draw(background);

        sf::Text lives(toSfString("Жизни: " + std::to_string(stats.lives) + "/" +
                                 std::to_string(stats.maxLives) + "   |   Длина: " +
                                 std::to_string(stats.length)), font, 20);
        lives.setFillColor(Palette::Accent);
        lives.setPosition(20.0f, 10.0f);
        window.draw(lives);

        sf::Text score(toSfString("Очки: " + std::to_string(stats.score)), font, 20);
        score.setPosition(20.0f, 38.0f);
        window.draw(score);

        sf::Text level(toSfString("Уровень: " + std::to_string(stats.level) + " / 10"), font, 21);
        centerTextX(level, 570.0f, 10.0f);
        window.draw(level);

        sf::Text food(toSfString("Осталось собрать: " + std::to_string(stats.foodLeft)), font, 16);
        food.setFillColor(Palette::TextMuted);
        centerTextX(food, 570.0f, 40.0f);
        window.draw(food);
    }
};

class UIManager {
private:
    sf::RenderWindow& window;
    sf::Font font;
    UIState state = UIState::MainMenu;
    GameStats stats;
    HUD hud;
    std::vector<Button> mainMenuButtons;
    std::vector<Button> pauseButtons;
    Button backToMenuBtn;

    bool loadSystemFont() {
        const std::vector<std::string> paths = {
            "arial.ttf", "font.ttf", "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf"
        };
        for (const auto& path : paths) {
            if (font.loadFromFile(path)) return true;
        }
        return false;
    }

    void drawText(const std::string& value, unsigned int size,
                  float y, sf::Color color = Palette::Text) {
        sf::Text text(toSfString(value), font, size);
        text.setFillColor(color);
        centerTextX(text, SCREEN_WIDTH / 2.0f, y);
        window.draw(text);
    }

public:
    GameStats& getStats() { return stats; }
    UIState getState() const { return state; }

    void startNewGame() {
        stats.reset();
        state = UIState::Playing;
    }

    void restartLevel() {
        // В автономной демонстрации перезапуск сбрасывает показатели уровня.
        startNewGame();
    }

    explicit UIManager(sf::RenderWindow& win)
        : window(win),
          backToMenuBtn(310.0f, 520.0f, 280.0f, 50.0f, "Вернуться в меню",
                        [this]() { state = UIState::MainMenu; }) {
        if (!loadSystemFont()) {
            throw std::runtime_error("Font not found. Place font.ttf beside the executable or in the working directory.");
        }
        mainMenuButtons = {
            Button(310.0f, 185.0f, 280.0f, 50.0f, "Новая игра",
                   [this]() { startNewGame(); }, Palette::Accent),
            Button(310.0f, 250.0f, 280.0f, 50.0f, "Управление",
                   [this]() { state = UIState::Controls; }, Palette::Accent),
            Button(310.0f, 315.0f, 280.0f, 50.0f, "Об игре",
                   [this]() { state = UIState::About; }),
            Button(310.0f, 380.0f, 280.0f, 50.0f, "Выход",
                   [this]() { window.close(); }, Palette::Panel, Palette::Danger)
        };
        pauseButtons = {
            Button(310.0f, 250.0f, 280.0f, 50.0f, "Продолжить",
                   [this]() { state = UIState::Playing; }, Palette::Accent),
            Button(310.0f, 315.0f, 280.0f, 50.0f, "Перезапустить уровень",
                   [this]() { restartLevel(); }),
            Button(310.0f, 380.0f, 280.0f, 50.0f, "Главное меню",
                   [this]() { state = UIState::MainMenu; })
        };
    }

    void handleEvent(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (state == UIState::Playing) state = UIState::Pause;
            else if (state == UIState::Pause) state = UIState::Playing;
            else state = UIState::MainMenu;
            return;
        }
        if (state == UIState::Playing) {
            // Тест панели HUD до подключения игровой логики.
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Enter && stats.foodLeft > 0) {
                stats.score += 25;
                stats.length += 1;
                stats.eatenTotal += 1;
                stats.foodLeft -= 1;
            }
            return;
        }
        if (state == UIState::Pause) {
            for (auto& button : pauseButtons) {
                if (button.handleEvent(event, window)) break;
            }
            return;
        }
        if (state == UIState::MainMenu) {
            for (auto& button : mainMenuButtons) {
                if (button.handleEvent(event, window)) break;
            }
        } else if (state == UIState::Controls || state == UIState::About) {
            backToMenuBtn.handleEvent(event, window);
        }
    }

    void draw() {
        window.clear(Palette::Bg);
        if (state == UIState::MainMenu) {
            drawText("SNAKE: OVERDRIVE", 44, 100.0f, Palette::Accent);
            drawText("Аркада на выживание и развитие змейки", 18, 165.0f, Palette::TextMuted);
            for (auto& button : mainMenuButtons) button.draw(window, font);
        } else if (state == UIState::Controls) {
            drawText("УПРАВЛЕНИЕ", 32, 90.0f, Palette::Accent);
            drawText("Движение: W, A, S, D или стрелки", 22, 210.0f);
            drawText("Фазовый щит: Пробел", 22, 270.0f);
            drawText("Ускорение: Shift", 22, 330.0f);
            drawText("Пауза в игре: Esc", 22, 390.0f);
            backToMenuBtn.draw(window, font);
        } else if (state == UIState::About) {
            drawText("ОБ ИГРЕ", 32, 90.0f, Palette::Accent);
            drawText("Snake: Overdrive", 28, 220.0f);
            drawText("Аркада на выживание и развитие змейки", 20, 285.0f);
            drawText("C++17 / SFML", 20, 345.0f, Palette::TextMuted);
            backToMenuBtn.draw(window, font);
        } else if (state == UIState::Playing || state == UIState::Pause) {
            hud.draw(window, font, stats);
            if (state == UIState::Playing) {
                drawText("Демонстрация интерфейса", 28, 200.0f, Palette::Accent);
                drawText("Enter — проверить счётчик еды", 20, 280.0f);
                drawText("Esc — пауза", 20, 330.0f, Palette::TextMuted);
            } else {
                drawText("ПАУЗА", 32, 150.0f, Palette::Accent);
                for (auto& button : pauseButtons) button.draw(window, font);
            }
        }
    }
};

int main() {
    try {
        sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT),
                                "Snake: Overdrive", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);
        window.setKeyRepeatEnabled(false);
        UIManager ui(window);
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close();
                else ui.handleEvent(event);
                if (!window.isOpen()) break;
            }
            if (!window.isOpen()) break;
            ui.draw();
            window.display();
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
