#include "Scene_Menu.h"
#include "Scene_Frogger.h"
#include <memory>

void Scene_Menu::onEnd()
{
	_menuStrings.clear();
	_levelPaths.clear();
}


Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}



void Scene_Menu::init()
{
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
	registerAction(sf::Keyboard::D, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");

	_title = "Go Safe!";
	_menuStrings.push_back("Level 1");
	_menuStrings.push_back("Level 2");
	_menuStrings.push_back("Level 3");

	_levelPaths.push_back("../assets/level1.txt");
	_levelPaths.push_back("../assets/level1.txt");
	_levelPaths.push_back("../assets/level1.txt");

	_menuText.setFont(Assets::getInstance().getFont("main"));

	const size_t CHAR_SIZE{ 64 };
	_menuText.setCharacterSize(CHAR_SIZE);

}

void Scene_Menu::update(sf::Time dt)
{
	_entityManager.update();
}


void Scene_Menu::sRender()
{
    _game->window().clear(sf::Color(10, 10, 40));

    sf::Vector2u winSize = _game->window().getSize();

    sf::Text titleText;
    titleText.setFont(Assets::getInstance().getFont("main"));
    titleText.setString(_title); // _title is set during init, e.g., "GEX Planes"
    titleText.setCharacterSize(64);
    titleText.setFillColor(sf::Color::White);

    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
    titleText.setPosition(winSize.x / 2.f, winSize.y * 0.2f);

    sf::Text titleShadow = titleText;
    titleShadow.setFillColor(sf::Color(0, 0, 0, 150));
    titleShadow.setPosition(titleText.getPosition() + sf::Vector2f(3.f, 3.f));
    _game->window().draw(titleShadow);
    _game->window().draw(titleText);

    float startY = winSize.y * 0.4f;
    float optionSpacing = 80.f;

    for (size_t i = 0; i < _menuStrings.size(); ++i)
    {
        sf::Text optionText;
        optionText.setFont(Assets::getInstance().getFont("main"));
        optionText.setString(_menuStrings.at(i));
        optionText.setCharacterSize(48);

        if (i == _menuIndex)
            optionText.setFillColor(sf::Color::Yellow);
        else
            optionText.setFillColor(sf::Color(200, 200, 200));

        sf::FloatRect textBounds = optionText.getLocalBounds();
        optionText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        optionText.setPosition(winSize.x / 2.f, startY + i * optionSpacing);

        sf::Text optionShadow = optionText;
        optionShadow.setFillColor(sf::Color(0, 0, 0, 150));
        optionShadow.setPosition(optionText.getPosition() + sf::Vector2f(2.f, 2.f));
        _game->window().draw(optionShadow);

        _game->window().draw(optionText);
    }

    sf::Text footer;
    footer.setFont(Assets::getInstance().getFont("main"));
    footer.setString("UP: W    DOWN: S   PLAY: D    QUIT: ESC");
    footer.setCharacterSize(20);
    footer.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect footerBounds = footer.getLocalBounds();
    footer.setOrigin(footerBounds.left + footerBounds.width / 2.f, footerBounds.top + footerBounds.height / 2.f);
    footer.setPosition(winSize.x / 2.f, winSize.y * 0.85f);
    _game->window().draw(footer);

    //_game->window().display();
}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			_menuIndex = (_menuIndex + _menuStrings.size() - 1) % _menuStrings.size();
		}
		else if (action.name() == "DOWN")
		{
			_menuIndex = (_menuIndex + 1) % _menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			std::cout << "Starting Game..." << std::endl;
			_game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, _levelPaths[_menuIndex]), true);
		}
		else if (action.name() == "QUIT")
		{
			_game->quit();
		}
	}
}