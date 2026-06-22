#ifndef USERDICT_H
#define USERDICT_H

#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

namespace freewb
{

/** 用户词库：~/.local/freewb/data/user_word.txt，含 [UserWord] 与 [DeletedWord] 两段。 */
class UserDict
{
public:
    UserDict();
    ~UserDict();

    void reload();
    bool save() const;

    bool contains(const std::string &code) const;
    bool hasUserEntry(const std::string &code, const std::string &text) const;
    bool isDeleted(const std::string &code, const std::string &text) const;

    /** 某词条编码 key 以 @p prefix 为前缀或等于 prefix（超长拆码 bhasMatch）。 */
    bool hasEntryStartingWithPrefix(const std::string &prefix) const;
    void appendCandidatesForPrefix(const std::string &prefix, CandidatePayload &out) const;

    bool addUserEntry(const std::string &code, const std::string &text);
    bool removeUserEntry(const std::string &code, const std::string &text);
    bool markDeleted(const std::string &code, const std::string &text);
    bool removeDeletedEntry(const std::string &code, const std::string &text);

private:
    enum class Section
    {
        UserWord,
        DeletedWord,
    };

    std::string filePath() const;
    static std::string trim(const std::string &s);
    static bool appendUnique(std::vector<std::string> &list, const std::string &text);
    static bool removeExact(std::vector<std::string> &list, const std::string &text);
    bool parseFile(const std::string &filePath);
    static void appendSectionEntries(std::ostream &out, Section section,
                                     const std::unordered_map<std::string, std::vector<std::string>> &entries);

private:
    std::string filePath_;
    std::unordered_map<std::string, std::vector<std::string>> userEntries_;
    std::unordered_map<std::string, std::vector<std::string>> deletedEntries_;
};

} // namespace freewb

#endif // USERDICT_H
