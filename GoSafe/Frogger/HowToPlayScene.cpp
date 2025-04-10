#include "HowToPlayScene.h"
#include "Scene_Menu.h"
#include "Leaf.h"
#include "Assets.h"
#include "Command.h"
#include <iostream>
#include <cstdlib>
#include <memory>

HowToPlayScene::HowToPlayScene(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    initLeaves();
}

void HowToPlayScene::initLeaves()
{
    const int leafCount = 20; 
    fallingLeaves.clear();

    const sf::Texture& leafTexture = Assets::getInstance().getTexture("leaf");

    sf::Vector2u winSize = _game->window().getSize();
    for (int i = 0; i < leafCount; i++) {
        Leaf leaf;
        leaf.sprite.setTexture(leafTexture);

        float x = static_cast<float>(std::rand() % winSize.x);
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

void HowToPlayScene::update(sf::Time dt)
{
    updateLeaves(dt);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
        _game->changeScene("MENU", std::make_shared<Scene_Menu>(_game), true);
    }
}

void HowToPlayScene::updateLeaves(sf::Time dt)
{
    sf::Vector2u winSize = _game->window().getSize();
    for (auto& leaf : fallingLeaves)
    {
        leaf.sprite.move(leaf.drift * dt.asSeconds(), leaf.fallSpeed * dt.asSeconds());

        sf::Vector2f pos = leaf.sprite.getPosition();
        if (pos.y > winSize.y)
        {
            pos.y = -leaf.sprite.getLocalBounds().height;
            pos.x = static_cast<float>(std::rand() % winSize.x);
            leaf.sprite.setPosition(pos);
        }
    }
}



void HowToPlayScene::renderLeaves()
{
    for (const auto& leaf : fallingLeaves)
    {
        _game->window().draw(leaf.sprite);
    }
}

void HowToPlayScene::sRender()
{
    _game->window().clear(sf::Color(10, 10, 40));
    sf::Vector2u winSize = _game->window().getSize();

    renderLeaves();

    // --- Draw the "How To Play" Title ---
    sf::Text title;
    title.setFont(Assets::getInstance().getFont("main"));
    title.setString("How To Play");
    title.setCharacterSize(80); // Increased title size
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold); // Make text bold
    title.setOutlineThickness(2.f); // Add an outline for extra contrast
    title.setOutlineColor(sf::Color::Black);

    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.left + titleBounds.width / 2.f,
        titleBounds.top + titleBounds.height / 2.f);
    title.setPosition(winSize.x / 2.f, winSize.y / 6.f);
    _game->window().draw(title);

    sf::Text instructions;
    instructions.setFont(Assets::getInstance().getFont("main"));
    instructions.setCharacterSize(40); // Increased instruction font size
    instructions.setFillColor(sf::Color::White);
    instructions.setStyle(sf::Text::Bold);
    instructions.setOutlineThickness(1.5f);
    instructions.setOutlineColor(sf::Color::Black);

    std::string instrText =
        " Use W, A, S, D or Arrow keys to move.\n\n"
        " Press Space to jump on the boat.\n\n"
        " Avoid enemy cars and river enemies as well as river.\n\n"
        " Drones will attack with lasers; avoid them.\n\n"
        " Traffic Signals: Press E to toggle state.\n\n"
        " When Red, press X for safe passage.\n\n"
        " Reach the top to win the game.\n\n"
        " Press ESC to return to Main Menu.";
    instructions.setString(instrText);

    sf::FloatRect instBounds = instructions.getLocalBounds();
    instructions.setOrigin(instBounds.left + instBounds.width / 2.f,
        instBounds.top + instBounds.height / 2.f);
    instructions.setPosition(winSize.x / 2.f, winSize.y / 2.f);
    _game->window().draw(instructions);
}

void HowToPlayScene::sDoAction(const Command& action)
{
    if (action.type() == "QUIT")
        _game->changeScene("MENU", std::make_shared<Scene_Menu>(_game), true);
}

void HowToPlayScene::onEnd()
{
    std::cout << "Exiting How To Play Screen..." << std::endl;
}
