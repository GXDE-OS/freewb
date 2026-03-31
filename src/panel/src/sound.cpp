#include "sound.h"

#include "commdefine.h"

#define SOUND_FILE_PATH "/usr/share/freewb/sound"

const char *Sound::s_soundData[SOUND_NUM] = {
    "letter.wav", // SOUND_LETTER
    "enter.wav",  // SOUND_ENTER
    "back.wav",   // SOUND_BACK
    "recode.wav", // SOUND_RECODE
    "space.wav",  // SOUND_SAPCE
    "empty.wav"   // SOUND_EMPTY
};

Sound::Sound(QObject *parent) : QObject(parent)
{
}

void Sound::play_sound(SoundType soundType)
{
    if (soundType > SOUND_NUM)
    {
        qWarning() << "have no sound idx: " << SOUND_NUM;
        return;
    }

#if 1
    char cmd[128] = {0};
    sprintf(cmd, "aplay %s%s%s > /dev/null 2>&1 &", SOUND_FILE_PATH, "/", s_soundData[soundType]);
    system(cmd);
#else
    QProcess process;
    QStringList args;
    args << QString(SOUND_FILE_PATH) + "/" + s_soundData[soundType];
    process.startDetached("aplay", args);
#endif
}
