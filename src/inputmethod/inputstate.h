#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <fcitx/ime.h>

typedef enum _KEY_RELEASED {
    KR_OTHER = 0,
    KR_SWITCH,
    KR_2ND_SELECTKEY,
    KR_3RD_SELECTKEY,
    KR_SWITCH_IM,
    KR_SWITCH_IM_REVERSE,
    KR_TRIGGER,
    KR_ACTIVATE,
    KR_DEACTIVATE
} KEY_RELEASED;

struct _FcitxInputState {
    long unsigned int lastKeyPressedTime;
    boolean bIsDoInputOnly;
    KEY_RELEASED keyReleased;
    int iCodeInputCount;
    char strCodeInput[MAX_USER_INPUT * UTF8_MAX_LENGTH + 1];
    char strStringGet[MAX_USER_INPUT * UTF8_MAX_LENGTH + 1];
    char strLastCommit[MAX_USER_INPUT * UTF8_MAX_LENGTH + 1];
    boolean bIsInRemind;

    time_t dummy;
    int iCursorPos;
    int iClientCursorPos;
    boolean bShowCursor;
    int dummy2;
    int lastIsSingleHZ;
    boolean bLastIsNumber;
    boolean dummy3;

    /* the ui message part, if there is something in it, then it will be shown */
    struct _FcitxCandidateWordList* candList;
    FcitxMessages* msgPreedit;
    FcitxMessages* msgAuxUp;
    FcitxMessages* msgAuxDown;
    FcitxMessages* msgClientPreedit;

    uint32_t keycode;
    uint32_t keysym;
    uint32_t keystate;

    int padding[60];
};

void FreeWubiInputStateInitializeInstance();
void FreeWubiInputStateDestroyInstance();
FcitxInputState* FreeWubiGetInputState();

void FreeWubiInputStateCleanInputWindow(FcitxInputState *inputState);
void FreeWubiInputStateCleanInputWindowUp(FcitxInputState *inputState);
void FreeWubiInputStateCleanInputWindowDown(FcitxInputState* inputState);

void FreeWubiInputStateResetRawInputBuffer();

struct _FcitxCandidateWordList* FreeWubiInputStateGetCandidateList(FcitxInputState* input);

void FreeWubiResetInputState(FcitxInputState* input);

#endif