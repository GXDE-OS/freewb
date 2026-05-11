#include "userdict.h"

#include <sys/stat.h>

#include <fstream>

#include "utils.h"

namespace freewb
{

UserDict::UserDict() : filePath_(filePath())
{
    reload();
}

UserDict::~UserDict() = default;

void UserDict::reload()
{
    entries_.clear();
    if (filePath_.empty())
    {
        return;
    }

    parseFile(filePath_);
}

bool UserDict::contains(const std::string &code) const
{
    return entries_.find(code) != entries_.end();
}

bool UserDict::hasEntryStartingWithPrefix(const std::string &prefix) const
{
    if (prefix.empty())
    {
        return false;
    }
    if (contains(prefix))
    {
        return true;
    }
    for (const auto &kv : entries_)
    {
        const std::string &key = kv.first;
        if (key.size() < prefix.size())
        {
            continue;
        }
        if (key.compare(0, prefix.size(), prefix) != 0)
        {
            continue;
        }
        for (const auto &hz : kv.second)
        {
            if (!hz.empty())
            {
                return true;
            }
        }
    }
    return false;
}

const std::vector<std::string> &UserDict::lookup(const std::string &code) const
{
    auto it = entries_.find(code);
    if (it == entries_.end())
    {
        return emptyTexts_;
    }
    return it->second;
}

std::string UserDict::filePath() const
{
    return userFreewbPath() + "/data/user_word.txt";
}

std::string UserDict::trim(const std::string &s)
{
    std::size_t first = 0;
    while (first < s.size() && (s[first] == ' ' || s[first] == '\t' || s[first] == '\r' || s[first] == '\n'))
    {
        ++first;
    }
    std::size_t last = s.size();
    while (last > first && (s[last - 1] == ' ' || s[last - 1] == '\t' || s[last - 1] == '\r' || s[last - 1] == '\n'))
    {
        --last;
    }
    return s.substr(first, last - first);
}

bool UserDict::parseFile(const std::string &filePath)
{
    std::ifstream in(filePath);
    if (!in)
    {
        return false;
    }

    std::string line;
    while (std::getline(in, line))
    {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == '[')
        {
            continue;
        }

        const std::size_t eq = trimmed.find('=');
        if (eq == std::string::npos || eq == 0)
        {
            continue;
        }

        const std::string code = trim(trimmed.substr(0, eq));
        const std::string text = trim(trimmed.substr(eq + 1));
        if (code.empty() || text.empty())
        {
            continue;
        }
        entries_[code].push_back(text);
    }
    return true;
}

} // namespace freewb
