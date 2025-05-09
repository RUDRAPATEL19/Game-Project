#include "GameEngine.h"
#include "Assets.h"
#include "Scene_Frogger.h"
#include "Scene_Menu.h"
#include "Command.h"
#include <fstream>
#include <memory>
#include <cstdlib>
#include <iostream>
#include "MusicPlayer.h"
#include <SFML/Graphics.hpp>


GameEngine::GameEngine(const std::string& path)
{
    Assets::getInstance().loadFromFile("../config.txt") ;
	init(path);
}

bool GameEngine::hasScene(const std::string& name) const
{
    return m_sceneMap.find(name) != m_sceneMap.end();
}


void GameEngine::resumeScene()
{
    if (!_pausedSceneName.empty() && _sceneMap.contains(_pausedSceneName))
    {
        _currentScene = _pausedSceneName;
        _pausedSceneName = "";
    }
}



void GameEngine::init(const std::string& path)
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    _window.create(desktop, "Go Safe", sf::Style::Fullscreen);

    MusicPlayer::getInstance().addSong("hop", "../assets/Sound/jump.mp3");
    MusicPlayer::getInstance().addSong("run", "../assets/Sound/run.mp3");

    if (!backgroundMusic.openFromFile("../assets/Music/gosafe.wav"))
    {
        std::cerr << "Error: Could not load background music!" << std::endl;
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(20.f); // Adjust volume as desired.
    backgroundMusic.play();


    unsigned int designWidth = 2560;
    unsigned int designHeight = 1600;

    sf::View view(sf::FloatRect(0.f, 0.f, designWidth, designHeight));
    _window.setView(view);

    std::cout << "Desktop resolution: " << desktop.width << "x" << desktop.height << std::endl;
    std::cout << "Design resolution: " << designWidth << "x" << designHeight << std::endl;

    _statisticsText.setFont(Assets::getInstance().getFont("main"));
    _statisticsText.setPosition(15.f, 5.f);
    _statisticsText.setCharacterSize(15);

    // Change to the menu scene.
    changeScene("MENU", std::make_shared<Scene_Menu>(this));

}

void GameEngine::loadConfigFromFile(const std::string &path, unsigned int &width, unsigned int &height) const {
    std::ifstream config(path);
    if (config.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        config.close();
        exit(1);
    }
    std::string token{""};
    config >> token;
    while (!config.eof()) {
        if (token == "Window") {
            config >> width >> height;
        } else if (token[0] == '#') {
            std::string tmp;
            std::getline(config, tmp);
            std::cout << tmp << "\n";
        }

        if (config.fail()) {
            config.clear(); // clear error on stream
            std::cout << "*** Error reading config file\n";
        }
        config >> token;
    }
    config.close();
}


void GameEngine::sUserInput()
{
	sf::Event event;
	while (_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)  
			quit();  

        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::Enter:
                currentScene()->doAction(Command("Enter", "SELECT"));
                break;
            case sf::Keyboard::Escape:
                currentScene()->doAction(Command("Escape", "QUIT"));
                break;
            default:
                break;
            }
        }

	}
}


std::shared_ptr<Scene> GameEngine::currentScene()
{
	return _sceneMap.at(_currentScene);
}

void GameEngine::changeScene(const std::string& sceneName,
    std::shared_ptr<Scene> scene,
    bool endCurrentScene)
{
    if (_currentScene != "MENU" && sceneName == "MENU")
    {
        _pausedSceneName = _currentScene;
    }

    if (endCurrentScene)
    {
        _sceneMap.clear();
    }

    if (scene != nullptr)
    {
        _sceneMap[sceneName] = scene;
    }

    _currentScene = sceneName;

    if (_sceneMap.contains(_currentScene) && _sceneMap[_currentScene] != nullptr)
    {
        _window.clear();
        _sceneMap[_currentScene]->sRender();
        _window.display();
    }
}





void GameEngine::quit()
{
	_window.close();
}


void GameEngine::run()
{
	const sf::Time SPF = sf::seconds(1.0f / 60.f); 

	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	while (isRunning())
	{
		sUserInput();								

		timeSinceLastUpdate += clock.restart();		 
		while (timeSinceLastUpdate > SPF)
		{
			currentScene()->update(SPF);			
			timeSinceLastUpdate -= SPF;
		}

		currentScene()->sRender();	

		// draw stats

		// display
		window().display();
	}
}

void GameEngine::quitLevel() {
    changeScene("MENU", std::make_shared<Scene_Menu>(this), true);
}


void GameEngine::backLevel() {
	changeScene("MENU", nullptr, false);

}


sf::RenderWindow& GameEngine::window()
{
	return _window;
}

sf::Vector2f GameEngine::windowSize() const {
	return sf::Vector2f{_window.getSize()};
}


bool GameEngine::isRunning()
{
	return (_running && _window.isOpen());
}
