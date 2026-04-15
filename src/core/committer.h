#ifndef COMMITER_H
#define COMMITER_H

#include <string>

#include "candidatelist.h"
#include "enginemanager.h"
#include "key.h"
#include "sdbus_proxy.h"

namespace freewb
{
class Committer
{
public:
    Committer(CommitCallback commitCallback, CandidateList *candidateList, EngineManager *engineManager);
    ~Committer();

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    const std::string &lastCommitString() const;

private:
    void loadSettings();

private:
    CandidateList *candidateList_;
    EngineManager *engineManager_;

    CommitCallback commitCallback_;

    FreewbKeySym secondRecodeKey_;
    FreewbKeySym thirdRecodeKey_;
    FreewbKeySym prevPageKey_;
    FreewbKeySym nextPageKey_;

    std::string lastCommitString_;
};
} // namespace freewb

#endif // COMMITER_H