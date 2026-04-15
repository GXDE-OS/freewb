#include <iostream>

#include "sound.h"

int main()
{
    std::cout << "freewb-sound-test: Sound::play (each SoundType)\n";

    Sound::play(SOUND_LETTER);
    Sound::play(SOUND_ENTER);
    Sound::play(SOUND_BACK);
    Sound::play(SOUND_RECODE);
    Sound::play(SOUND_SAPCE);
    Sound::play(SOUND_EMPTY);

    std::cout << "sound_test: play calls finished (aplay runs in background)\n";
    return 0;
}
