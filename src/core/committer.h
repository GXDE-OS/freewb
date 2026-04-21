#ifndef COMMITTER_H
#define COMMITTER_H

#include <string>

#include "candidatelist.h"
#include "enginemanager.h"
#include "key.h"

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
    void commit(const std::string &text);

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

#endif // COMMITTER_H