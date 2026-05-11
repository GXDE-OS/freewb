#ifndef COMMITTER_H
#define COMMITTER_H

#include <string>

#include "key.h"
#include "types.h"

namespace freewb
{

class Freewb;

class Committer
{
public:
    Committer(CommitCallback commitCallback, Freewb *freewb);
    ~Committer();

    bool processKey(FreewbKeySym keysym, FreewbKeyState state);
    const std::string &lastCommitString() const;
    void commitFirstCandidate();
    bool tryExactDictionarySingleCandidateCommit();

private:
    void loadSettings();
    void connectDBusCallback();
    void commit(const std::string &text);

private:
    Freewb *freewb_;

    CommitCallback commitCallback_;

    FreewbKeySym secondRecodeKey_;
    FreewbKeySym thirdRecodeKey_;
    FreewbKeySym prevPageKey_;
    FreewbKeySym nextPageKey_;

    std::string lastCommitString_;
};
} // namespace freewb

#endif // COMMITTER_H