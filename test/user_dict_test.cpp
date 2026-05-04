/**
 * UserDict 测试：通过 setenv("HOME", tmpdir) 把 userFreewbPath() 导向一个临时目录，
 * 在 "$HOME/.local/freewb/data/user_word.txt" 下写入不同格式的样本，
 * 验证加载、查询、重新加载、缺文件等行为。
 *
 * 刻意使用 C++11 + POSIX，不依赖 std::filesystem（C++17）。
 */

#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "userdict.h"

namespace
{
int g_failed = 0;

#define EXPECT(cond)                                                                                                             \
    do                                                                                                                           \
    {                                                                                                                            \
        if (!(cond))                                                                                                             \
        {                                                                                                                        \
            ++g_failed;                                                                                                          \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " " << #cond << std::endl;                                   \
        }                                                                                                                        \
    } while (0)

int removeEntry(const char *path, const struct stat * /*sb*/, int typeflag, struct FTW * /*ftwbuf*/)
{
    if (typeflag == FTW_DP)
    {
        return ::rmdir(path);
    }
    return ::remove(path);
}

void removeRecursively(const std::string &path)
{
    ::nftw(path.c_str(), removeEntry, 16, FTW_DEPTH | FTW_PHYS);
}

bool makeDirs(const std::string &path)
{
    std::string acc;
    for (std::size_t i = 0; i < path.size(); ++i)
    {
        const char ch = path[i];
        if (ch == '/' && !acc.empty())
        {
            if (::mkdir(acc.c_str(), 0755) != 0 && errno != EEXIST)
            {
                return false;
            }
        }
        acc.push_back(ch);
    }
    if (!acc.empty() && ::mkdir(acc.c_str(), 0755) != 0 && errno != EEXIST)
    {
        return false;
    }
    return true;
}

bool writeFile(const std::string &path, const std::string &content)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        return false;
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    return static_cast<bool>(out);
}

std::string makeTempHome()
{
    char tmpl[] = "/tmp/freewb-userdict-test-XXXXXX";
    const char *dir = ::mkdtemp(tmpl);
    if (dir == nullptr)
    {
        std::cerr << "mkdtemp failed: " << std::strerror(errno) << std::endl;
        std::exit(2);
    }
    return std::string(dir);
}

std::string userWordPath(const std::string &home)
{
    return home + "/.local/freewb/data/user_word.txt";
}

bool prepareUserWord(const std::string &home, const std::string &content)
{
    const std::string path = userWordPath(home);
    if (!makeDirs(home + "/.local/freewb/data"))
    {
        return false;
    }
    return writeFile(path, content);
}

void testMissingFile()
{
    const std::string home = makeTempHome();
    ::setenv("HOME", home.c_str(), 1);

    freewb::UserDict dict;
    EXPECT(!dict.contains("anything"));
    EXPECT(dict.lookup("anything").empty());

    removeRecursively(home);
}

void testBasicParse()
{
    const std::string home = makeTempHome();
    ::setenv("HOME", home.c_str(), 1);

    const std::string content = "[UserWord]\n"
                                "date=$Y年$M月$D日\n"
                                "date=$y年$m月$d日\n"
                                "joke=hello\n";
    EXPECT(prepareUserWord(home, content));

    freewb::UserDict dict;

    EXPECT(dict.contains("date"));
    const auto &dateValues = dict.lookup("date");
    EXPECT(dateValues.size() == 2);
    if (dateValues.size() == 2)
    {
        EXPECT(dateValues[0] == "$Y年$M月$D日");
        EXPECT(dateValues[1] == "$y年$m月$d日");
    }

    EXPECT(dict.contains("joke"));
    const auto &jokeValues = dict.lookup("joke");
    EXPECT(jokeValues.size() == 1);
    if (!jokeValues.empty())
    {
        EXPECT(jokeValues[0] == "hello");
    }

    EXPECT(!dict.contains("missing"));
    EXPECT(dict.lookup("missing").empty());

    removeRecursively(home);
}

void testTolerantParse()
{
    const std::string home = makeTempHome();
    ::setenv("HOME", home.c_str(), 1);

    // 无 [UserWord] 头、夹杂空行/注释/非法行，同样应解析合法行。
    const std::string content = "\n"
                                "# this is a comment\n"
                                "  \t\n"
                                "nohead=ok\n"
                                "bad line without eq\n"
                                "= empty code\n"
                                "spaced  =  trimmed  \n";
    EXPECT(prepareUserWord(home, content));

    freewb::UserDict dict;

    EXPECT(dict.contains("nohead"));
    EXPECT(dict.lookup("nohead").size() == 1 && dict.lookup("nohead")[0] == "ok");

    EXPECT(dict.contains("spaced"));
    EXPECT(dict.lookup("spaced").size() == 1 && dict.lookup("spaced")[0] == "trimmed");

    EXPECT(!dict.contains(""));
    EXPECT(!dict.contains("bad line without eq"));

    removeRecursively(home);
}

void testReload()
{
    const std::string home = makeTempHome();
    ::setenv("HOME", home.c_str(), 1);

    EXPECT(prepareUserWord(home, "[UserWord]\nfoo=one\n"));
    freewb::UserDict dict;
    EXPECT(dict.contains("foo"));
    EXPECT(dict.lookup("foo").size() == 1);

    EXPECT(prepareUserWord(home, "[UserWord]\nfoo=one\nfoo=two\nbar=baz\n"));
    dict.reload();

    EXPECT(dict.contains("foo"));
    EXPECT(dict.lookup("foo").size() == 2);
    EXPECT(dict.contains("bar"));
    EXPECT(dict.lookup("bar").size() == 1 && dict.lookup("bar")[0] == "baz");

    removeRecursively(home);
}

} // namespace

int main()
{
    testMissingFile();
    testBasicParse();
    testTolerantParse();
    testReload();

    if (g_failed != 0)
    {
        std::cerr << "freewb-user-dict-test: " << g_failed << " assertion(s) failed\n";
        return 1;
    }
    std::cout << "freewb-user-dict-test: ok\n";
    return 0;
}
