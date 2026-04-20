#ifndef USERDICT_H
#define USERDICT_H

#include <string>
#include <unordered_map>
#include <vector>

namespace freewb
{
class UserDict
{
public:
    UserDict();
    ~UserDict();

    void reload();
    bool contains(const std::string &code) const;
    const std::vector<std::string> &lookup(const std::string &code) const;

private:
    std::string filePath() const;
    static std::string trim(const std::string &s);
    bool parseFile(const std::string &filePath);

private:
    std::string filePath_;
    std::unordered_map<std::string, std::vector<std::string>> entries_;
    std::vector<std::string> emptyTexts_;
};
} // namespace freewb

#endif // USERDICT_H
