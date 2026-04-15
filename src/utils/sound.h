#ifndef SOUND_H
#define SOUND_H

enum SoundType
{
    SOUND_LETTER,
    SOUND_ENTER,
    SOUND_BACK,
    SOUND_RECODE,
    SOUND_SAPCE,
    SOUND_EMPTY,
};

class Sound
{
public:
    static void play(SoundType soundType);

private:
    static const char *s_soundData[6];
};

#endif // SOUND_H
