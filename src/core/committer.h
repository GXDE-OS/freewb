#ifndef COMMITTER_H
#define COMMITTER_H

#include <cstddef>
#include <string>
#include <vector>

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
    std::string committedText(std::size_t charCount) const;

    void commit(const std::string &text);
    bool selectCandidate(int index);
    void loadSettings();

private:
    void appendCommittedText(const std::string &text);

private:
    Freewb *freewb_;

    CommitCallback commitCallback_;

    FreewbKeySym secondRecodeKey_;
    FreewbKeySym thirdRecodeKey_;
    FreewbKeySym prevPageKey_;
    FreewbKeySym nextPageKey_;

    std::string lastCommitString_;
    std::vector<std::string> committedTexts_;

    static constexpr std::size_t kMaxCommittedTextRecords = 128;
};
} // namespace freewb

#endif // COMMITTER_H