#include "Scene_Menu.h"
#include "Scene_Frogger.h"
#include "HowToPlayScene.h"
#include "Assets.h"
#include "Command.h"
#include "Leaf.h"
#include <iostream>
#include <cstdlib>
#include <memory>

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
    initLeaves();
}

void Scene_Menu::init()
{
    // Register keys for menu navigation.
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");
    registerAction(sf::Keyboard::D, "SELECT");
    registerAction(sf::Keyboard::A, "SELECT");
    registerAction(sf::Keyboard::Escape, "QUIT");

    _title = "Go Safe!";

    _menuStrings.clear();
    _menuStrings.push_back("Level 1");
    _menuStrings.push_back("Level 2");
    _menuStrings.push_back("How To Play");

    _levelPaths.clear();
    _levelPaths.push_back("../assets/level1.txt");
    _levelPaths.push_back("../assets/level2.txt");

    sf::Vector2u winSize = _game->window().getSize();
    titleFontSize = 80;
    optionFontSize = 48;
    infoFontSize = 32;

    std::cout << "Menu initialized with window size: " << winSize.x << "x" << winSize.y << std::endl;
}

void Scene_Menu::onEnd()
{
    _menuStrings.clear();
    _levelPaths.clear();
}

// --- Falling Leaves Functions ---
void Scene_Menu::initLeaves()
{
    const int leafCount = 20;
    fallingLeaves.clear();

    const sf::Texture& leafTexture = Assets::getInstance().getTexture("leaf");

    sf::Vector2u winSize = _game->window().getSize();
    for (int i = 0; i < leafCount; i++)
    {
        Leaf leaf;
        leaf.sprite.setTexture(leafTexture);

        float x = static_cast<float>(std::rand() % static_cast<int>(winSize.x));
        int randValue = std::rand() % static_cast<int>(winSize.y);
        float y = -static_cast<float>(randValue);
        leaf.sprite.setPosition(x, y);

        float scale = 0.5f + static_cast<float>(std::rand() % 100) / 200.f;
        leaf.sprite.setScale(scale, scale);

        leaf.fallSpeed = 30.f + static_cast<float>(std::rand() % 50);
        leaf.drift = static_cast<float>((std::rand() % 41) - 20);

        fallingLeaves.push_back(leaf);
    }
}

void Scene_Menu::updateLeaves(sf::Time dt)
{
    sf::Vector2u winSize = _game->window().getSize();
    for (auto& leaf : fallingLeaves)
    {
        leaf.sprite.move(leaf.drift * dt.asSeconds(), leaf.fallSpeed * dt.asSeconds());
        sf::Vector2f pos = leaf.sprite.getPosition();
        if (pos.y > static_cast<float>(winSize.y))
        {
            pos.y = -leaf.sprite.getLocalBounds().height;
            pos.x = static_cast<float>(std::rand() % static_cast<int>(winSize.x));
            leaf.sprite.setPosition(pos);
        }
    }
}

void Scene_Menu::renderLeaves()
{
    for (const auto& leaf : fallingLeaves)
        _game->window().draw(leaf.sprite);
}

void Scene_Menu::update(sf::Time dt)
{
    static float inputCooldown = 0.f;
    if (inputCooldown > 0.f)
        inputCooldown -= dt.asSeconds();

    if (inputCooldown <= 0.f)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            _menuIndex = (_menuIndex + _menuStrings.size() - 1) % _menuStrings.size();
            inputCooldown = 0.3f;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            _menuIndex = (_menuIndex + 1) % _menuStrings.size();
            inputCooldown = 0.3f;
        }
    }
    updateLeaves(dt); // Update leaf positions in addition to menu navigation.
}

void Scene_Menu::sRender()
{
    _game->window().clear(sf::Color(10, 10, 40));
    sf::Vector2u winSize = _game->window().getSize();
    renderLeaves();

    sf::Text titleText;
    titleText.setFont(Assets::getInstance().getFont("main"));
    titleText.setString(_title);
    titleText.setCharacterSize(titleFontSize);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);
    titleText.setOutlineThickness(2.f);
    titleText.setOutlineColor(sf::Color::Black);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.left + titleBounds.width / 2.f,
        titleBounds.top + titleBounds.height / 2.f);
    titleText.setPosition(winSize.x / 2.f, winSize.y / 4.f);
    _game->window().draw(titleText);

    float startY = winSize.y / 2.f;
    float spacing = optionFontSize * 2.0f;
    for (size_t i = 0; i < _menuStrings.size(); ++i)
    {
        sf::Text optionText;
        optionText.setFont(Assets::getInstance().getFont("main"));
        optionText.setString(_menuStrings[i]);
        optionText.setCharacterSize(optionFontSize);
        if (int(i) == _menuIndex)
            optionText.setFillColor(sf::Color::Yellow);
        else
            optionText.setFillColor(sf::Color(200, 200, 200));
        optionText.setStyle(sf::Text::Bold);
        optionText.setOutlineThickness(1.f);
        optionText.setOutlineColor(sf::Color::Black);
        sf::FloatRect optionBounds = optionText.getLocalBounds();
        optionText.setOrigin(optionBounds.left + optionBounds.width / 2.f,
            optionBounds.top + optionBounds.height / 2.f);
        optionText.setPosition(winSize.x / 2.f, startY + i * spacing);
        _game->window().draw(optionText);
    }

    sf::Text footer;
    footer.setFont(Assets::getInstance().getFont("main"));
    footer.setString("W/S: Navigate    D/A or Enter: Select    Esc: Quit");
    footer.setCharacterSize(infoFontSize);
    footer.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect footerBounds = footer.getLocalBounds();
    footer.setOrigin(footerBounds.left + footerBounds.width / 2.f,
        footerBounds.top + footerBounds.height / 2.f);
    footer.setPosition(winSize.x / 2.f, winSize.y * 0.9f);
    _game->window().draw(footer);
}

void Scene_Menu::sDoAction(const Command& action)
{
    if (action.type() == "QUIT")
    {
        _game->quit();
    }
    else if (action.type() == "SELECT")
    {
        if (_menuIndex == 0)
        {
            std::cout << "Level 1 selected!" << std::endl;
            _game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, _levelPaths[0]), true);
        }
        else if (_menuIndex == 1)
        {
            std::cout << "Level 2 selected!" << std::endl;
            _game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, _levelPaths[1]), true);
        }
        else if (_menuIndex == 2)
        {
            std::cout << "How To Play selected!" << std::endl;
            _game->changeScene("PLAY", std::make_shared<HowToPlayScene>(_game), true);
        }
    }
}
