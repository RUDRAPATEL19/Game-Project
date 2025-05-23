//
// Created by David Burchill on 2022-11-29.
//

#ifndef SFMLCLASS_MUSICPLAYER_H
#define SFMLCLASS_MUSICPLAYER_H

#include <map>
#include <string>
#include <SFML/Audio/Music.hpp>

using String = std::string;
class MusicPlayer
{

private:
    MusicPlayer();
    ~MusicPlayer() = default;

public:
    static MusicPlayer& getInstance();
    sf::SoundSource::Status getStatus() const { return _music.getStatus(); }
    MusicPlayer(const MusicPlayer&) = delete;
    MusicPlayer(MusicPlayer&&) = delete;
    MusicPlayer& operator=(const MusicPlayer&) = delete;
    MusicPlayer& operator=( MusicPlayer&&) = delete;

    void                            addSong(const std::string& name, const std::string& path );
    void							play(const std::string& theme, bool loop = true);
    void							stop();
    void							setPaused(bool paused);
    void							setVolume(float volume);


private:
    sf::Music						_music;
    std::map<String, String>	    _filenames;
    float							_volume{25};
};


#endif //SFMLCLASS_MUSICPLAYER_H
