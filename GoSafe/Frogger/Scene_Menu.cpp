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
    fromPausedGame = _game->fromPausedGameFlag;
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
    _levelPaths.clear();

    _title = "Go Safe!";

    // Show resume only if there is a paused scene
    if (fromPausedGame || !_game->_pausedSceneName.empty())
    {
        _menuStrings.push_back("Resume");
    }

    _menuStrings.push_back("Easy");
    _menuStrings.push_back("Hard");
    _menuStrings.push_back("How To Play");

    _levelPaths.push_back("../assets/level1.txt");
    _levelPaths.push_back("../assets/level2.txt");

    _menuIndex = 0;

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
    m_firstFrame = true;

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

void Scene_Menu::buildMenu()
{
    _menuStrings.clear();
    _levelPaths.clear();

    if (!_game->_pausedSceneName.empty())
    {
        _menuStrings.push_back("Resume");
    }

    if (!fromPausedGame)
    {
        _menuStrings.push_back("Easy");
        _menuStrings.push_back("Hard");
        _levelPaths.push_back("../assets/level1.txt");
        _levelPaths.push_back("../assets/level2.txt");
    }

    _menuStrings.push_back("How To Play");

    _menuIndex = 0;
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
    static bool firstFrame = true;
    if (m_firstFrame)
    {
        buildMenu();
        m_firstFrame = false;
    }


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
    //footer.setString("W/S: Navigate    Enter: Select    Esc: Quit");
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
    if (action.type() != "SELECT") return;

    const std::string& selectedOption = _menuStrings[_menuIndex];

    if (selectedOption == "Resume")
    {
        _game->changeScene(_game->_pausedSceneName, nullptr, false);
        _game->_pausedSceneName.clear();
        _game->fromPausedGameFlag = false;

    }
    else if (selectedOption == "Easy")
    {
        if (_levelPaths.size() >= 1)
        {
            _game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, _levelPaths[0]), true);
        }
    }
    else if (selectedOption == "Hard")
    {
        if (_levelPaths.size() >= 2)
        {
            _game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, _levelPaths[1]), true);
        }
    }
    else if (selectedOption == "How To Play")
    {
        _game->fromPausedGameFlag = true; 
        auto howToScene = std::make_shared<HowToPlayScene>(_game);
        _game->changeScene("HOWTO", howToScene, false); 
    }

}


