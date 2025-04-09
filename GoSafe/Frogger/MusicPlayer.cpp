//
// Created by David Burchill on 2022-11-29.
//

#include "MusicPlayer.h"
#include <stdexcept>
#include <iostream>


MusicPlayer::MusicPlayer() {
    _filenames["menuTheme"] = "../assets/Music/gosafe.wav";
    _filenames["gameTheme"] = "../assets/Music/gosafe.wav";
}

void MusicPlayer::addSong(const std::string&name, const std::string&path) {
    _filenames[name] = path;
}

MusicPlayer& MusicPlayer::getInstance() {
    static MusicPlayer instance;
    return instance;
}


void MusicPlayer::play(const std::string& theme, bool loop)
{
    std::string path = _filenames[theme];
    if (!_music.openFromFile(path))
        throw std::runtime_error("Music could not open file: " + path);

    _music.setVolume(_volume);
    _music.setLoop(loop);
    _music.play();
}




void MusicPlayer::stop() {
    _music.stop();
}


void MusicPlayer::setPaused(bool paused) {
    if (paused)
        _music.pause();
    else
        _music.play();
}


void MusicPlayer::setVolume(float volume) {
    _volume = volume;
    _music.setVolume(_volume);
}
