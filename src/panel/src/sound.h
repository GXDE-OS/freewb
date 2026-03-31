/****************************************************************************************
** @作者：lcj
**
** @说明：
**      Sound是一个从QObject类继承而来的非UI自定义类，其所有数据成员与成员函数皆为静态，该类设计目的为
**      为某些操作行为发出对应的提示音．
*************************************************************************************×**/

#ifndef SOUND_H
#define SOUND_H

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QSound>

enum SoundType
{
    SOUND_LETTER,
    SOUND_ENTER,
    SOUND_BACK,
    SOUND_RECODE,
    SOUND_SAPCE,
    SOUND_EMPTY,

    SOUND_NUM
};

class Sound : public QObject
{
    Q_OBJECT

public:
    explicit Sound(QObject *parent = nullptr);

public:
    static void play_sound(SoundType soundType);

private:
    static const char *s_soundData[SOUND_NUM];
};

#endif // SOUND_H
