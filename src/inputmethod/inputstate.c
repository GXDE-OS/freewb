#include "inputstate.h"

#include <fcitx-utils/utils.h>
#include <fcitx/candidate.h>
#include <fcitx/ui.h>

static FcitxInputState *freeWubiInputState = NULL;

FcitxInputState *FreeWubiGetInputState()
{
    return freeWubiInputState;
}

void FreeWubiInputStateInitializeInstance()
{
    if (freeWubiInputState != NULL)
    {
        FreeWubiInputStateDestroyInstance();
    }

    freeWubiInputState = fcitx_utils_malloc0(sizeof(FcitxInputState));

    freeWubiInputState->msgAuxUp = FcitxMessagesNew();
    freeWubiInputState->msgAuxDown = FcitxMessagesNew();
    freeWubiInputState->msgPreedit = FcitxMessagesNew();
    freeWubiInputState->msgClientPreedit = FcitxMessagesNew();
    freeWubiInputState->candList = FcitxCandidateWordNewList();
}

void FreeWubiInputStateDestroyInstance()
{
    if (freeWubiInputState == NULL)
    {
        return;
    }

    fcitx_utils_free(freeWubiInputState->msgAuxUp);
    fcitx_utils_free(freeWubiInputState->msgAuxDown);
    fcitx_utils_free(freeWubiInputState->msgPreedit);
    fcitx_utils_free(freeWubiInputState->msgClientPreedit);
    FcitxCandidateWordFreeList(freeWubiInputState->candList);

    fcitx_utils_free(freeWubiInputState);
    freeWubiInputState = NULL;
}

void FreeWubiInputStateCleanInputWindow(FcitxInputState *inputState)
{
    FreeWubiInputStateCleanInputWindowUp(inputState);
    FreeWubiInputStateCleanInputWindowDown(inputState);
}

void FreeWubiInputStateCleanInputWindowUp(FcitxInputState *inputState)
{
    FcitxMessagesSetMessageCount(inputState->msgAuxUp, 0);
    FcitxMessagesSetMessageCount(inputState->msgPreedit, 0);
    FcitxMessagesSetMessageCount(inputState->msgClientPreedit, 0);
}

void FreeWubiInputStateCleanInputWindowDown(FcitxInputState *inputState)
{
    FcitxCandidateWordReset(inputState->candList);
    FcitxMessagesSetMessageCount(inputState->msgAuxDown, 0);
}

void FreeWubiInputStateResetRawInputBuffer(FcitxInputState *inputState)
{
    FcitxInputStateSetRawInputBufferSize(inputState, 0);
    FcitxInputStateGetRawInputBuffer(inputState)[0] = '\0';
}

void FreeWubiResetInputState(FcitxInputState *input)
{
    FcitxCandidateWordReset(input->candList);
    input->iCursorPos = 0;
    input->iClientCursorPos = 0;

    FcitxInputStateGetRawInputBuffer(input)[0] = '\0';
    input->iCodeInputCount = 0;

    input->bIsDoInputOnly = false;
    input->bIsInRemind = false;

    FcitxMessages *msgAuxUp = FcitxInputStateGetAuxUp(input);
    FcitxMessagesSetMessageCount(msgAuxUp, 0);

    FcitxMessages *msgPreedit = FcitxInputStateGetPreedit(input);
    FcitxMessagesSetMessageCount(msgPreedit, 0);

    FcitxMessages *msgAuxDown = FcitxInputStateGetAuxDown(input);
    FcitxMessagesSetMessageCount(msgAuxDown, 0);

    FcitxCandidateWordReset(FreeWubiInputStateGetCandidateList(input));
    FcitxMessages *msgClientPreedit = FcitxInputStateGetClientPreedit(input);
}

struct _FcitxCandidateWordList *FreeWubiInputStateGetCandidateList(FcitxInputState *input)
{
    return input->candList;
}