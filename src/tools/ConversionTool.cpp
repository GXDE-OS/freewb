#include "ConversionTool.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"
#include "log.h"

namespace freewb::tools
{

constexpr char kUncommonPrefix = '~';
constexpr char kConstructPrefix = '^';

bool startsWith(const std::string &line, const char *prefix)
{
    return line.compare(0, std::string(prefix).size(), prefix) == 0;
}

std::string trimLine(std::string line)
{
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ' || line.back() == '\t'))
    {
        line.pop_back();
    }
    std::size_t start = 0;
    while (start < line.size() && (line[start] == ' ' || line[start] == '\t'))
    {
        ++start;
    }
    return line.substr(start);
}

bool parseDefaultRules(std::vector<EngineRuleBlock> &rules)
{
    static const char *const kRuleLines[] = {"e2=p11+p12+p21+p22", "e3=p11+p21+p31+p32", "a4=p11+p21+p31+n11"};
    constexpr uint32_t kCodeLength = 4;
    rules.assign(static_cast<size_t>(kCodeLength - 1U), EngineRuleBlock{});

    for (uint32_t ri = 0; ri < kCodeLength - 1U; ++ri)
    {
        std::string line = kRuleLines[ri];
        EngineRuleBlock &block = rules[ri];
        block.cells.resize(kCodeLength);

        const char flag = line[0];
        if (flag == 'e' || flag == 'E')
        {
            block.iFlag = 0;
        }
        else if (flag == 'a' || flag == 'A')
        {
            block.iFlag = 1;
        }
        else
        {
            return false;
        }

        const std::size_t eq = line.find('=');
        if (eq == std::string::npos || eq < 2)
        {
            return false;
        }
        block.iWords = static_cast<uint8_t>(std::stoi(line.substr(1, eq - 1)));

        std::string expr = line.substr(eq + 1);
        std::stringstream ss(expr);
        std::string token;
        for (uint32_t i = 0; i < kCodeLength; ++i)
        {
            if (!(ss >> token) || token.size() < 3)
            {
                return false;
            }
            const char cellFlag = token[0];
            if (cellFlag == 'p' || cellFlag == 'P')
            {
                block.cells[i].iFlag = 1;
            }
            else if (cellFlag == 'n' || cellFlag == 'N')
            {
                block.cells[i].iFlag = 0;
            }
            else
            {
                return false;
            }
            block.cells[i].iWhich = static_cast<uint8_t>(token[1] - '0');
            block.cells[i].iIndex = static_cast<uint8_t>(token[2] - '0');
        }
    }
    return true;
}

bool parseTextDictionary(const std::string &txtPath, MbDictionaryTable &table)
{
    std::ifstream in(txtPath);
    if (!in)
    {
        FREEWB_DEBUG("Cannot open source file {}.", txtPath);
        return false;
    }

    std::string tableName = "五笔字型";
    std::string tableInfo;
    std::string tableCreateTime = "2019-07-15 09:00";
    std::string endKeys;
    std::string specialKeys;
    std::string codeType;
    std::string straightUpKeys;
    std::string inputCode;
    uint8_t wildChar = 'z';
    uint8_t hasRule = 0;
    std::vector<EngineRuleBlock> rules;

    std::string line;
    bool inDataSection = false;
    while (std::getline(in, line))
    {
        line = trimLine(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }
        if (startsWith(line, "---") || startsWith(line, "★★★"))
        {
            continue;
        }
        if (startsWith(line, "[Description]"))
        {
            continue;
        }
        if (startsWith(line, "Name="))
        {
            tableName = line.substr(5);
            continue;
        }
        if (startsWith(line, "词库信息="))
        {
            tableInfo = line.substr(strlen("词库信息="));
            continue;
        }
        if (startsWith(line, "生成日期:"))
        {
            tableCreateTime = line.substr(strlen("生成日期:"));
            continue;
        }
        if (startsWith(line, "编码截止键="))
        {
            endKeys = line.substr(strlen("编码截止键="));
            continue;
        }
        if (startsWith(line, "特殊符号引导符="))
        {
            specialKeys = line.substr(strlen("特殊符号引导符="));
            continue;
        }
        if (startsWith(line, "编码方案类型="))
        {
            codeType = line.substr(strlen("编码方案类型="));
            continue;
        }
        if (startsWith(line, "径直上屏的标点="))
        {
            straightUpKeys = line.substr(strlen("径直上屏的标点="));
            continue;
        }
        if (startsWith(line, "UsedCodes="))
        {
            inputCode = line.substr(strlen("UsedCodes="));
            continue;
        }
        if (startsWith(line, "WildChar="))
        {
            if (line.size() > strlen("WildChar="))
            {
                wildChar = static_cast<uint8_t>(line[strlen("WildChar=")]);
            }
            continue;
        }
        if (startsWith(line, "[rule]"))
        {
            hasRule = 1;
            continue;
        }
        if (startsWith(line, "[Text]"))
        {
            inDataSection = true;
            break;
        }
    }

    if (!inDataSection || inputCode.empty())
    {
        FREEWB_DEBUG("Source File Format Error!");
        return false;
    }

    if (hasRule && !parseDefaultRules(rules))
    {
        FREEWB_DEBUG("Phrase rules are not suitable!");
        return false;
    }

    table.clear();
    table.setMetadata(tableName, tableInfo, tableCreateTime, endKeys, specialKeys, codeType, straightUpKeys, inputCode, wildChar,
                      hasRule, rules);

    while (std::getline(in, line))
    {
        line = trimLine(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::size_t split = 0;
        while (split < line.size() && !std::isspace(static_cast<unsigned char>(line[split])))
        {
            ++split;
        }
        if (split == 0 || split >= line.size())
        {
            continue;
        }

        const std::string code = line.substr(0, split);
        std::size_t pos = split;
        while (pos < line.size())
        {
            while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos])))
            {
                ++pos;
            }
            if (pos >= line.size())
            {
                break;
            }

            std::size_t start = pos;
            while (pos < line.size() && !std::isspace(static_cast<unsigned char>(line[pos])))
            {
                ++pos;
            }

            std::string text = line.substr(start, pos - start);
            if (!text.empty() && (text[0] == kUncommonPrefix || text[0] == kConstructPrefix))
            {
                text.erase(0, 1);
            }

            if (!text.empty())
            {
                table.addRecord(code, text);
            }
        }
    }

    return !table.records().empty();
}

bool exportTextDictionary(const std::string &txtPath, const MbDictionaryTable &table, int *hzCount)
{
    std::ofstream out(txtPath);
    if (!out)
    {
        FREEWB_DEBUG("Cannot create target file!");
        return false;
    }

    out << "[Description]\n";
    out << "Name=" << table.tableName() << "\n";
    out << "-----------------------------------------\n";
    out << "词库信息=" << table.tableInfo() << "\n";
    out << "生成日期:" << table.tableCreateTime() << "\n";
    out << "-----------------------------------------\n";
    out << "编码截止键=" << table.endKeys() << "\n";
    out << "特殊符号引导符=" << table.specialKeys() << "\n";
    out << "编码方案类型=" << table.codeType() << "\n";
    out << "径直上屏的标点=" << table.straightUpKeys() << "\n";
    out << "UsedCodes=" << table.strInputCode() << "\n";
    out << "WildChar=" << static_cast<char>(table.wildChar()) << "\n";
    out << "-----------------------------------------\n";
    out << "[rule]\n";
    out << "三字词=p11+p21+p31+p32\n";
    out << "-----------------------------------------\n";
    out << "~:生僻字词\n";
    out << "^:用户词组\n";
    out << "!联想词组\n";
    out << "★★★必须按编码顺序排列，否则词库会出错★★★\n";
    out << "-----------------------------------------\n";
    out << "[Text]\n";

    const auto &records = table.records();
    if (records.empty())
    {
        if (hzCount != nullptr)
        {
            *hzCount = 0;
        }
        return true;
    }

    std::string currentCode = records.front().first;
    out << currentCode;
    int count = 0;

    const auto writeRecord = [&](const std::pair<std::string, std::string> &record)
    {
        if (record.first != currentCode)
        {
            currentCode = record.first;
            out << '\n' << currentCode;
        }
        out << ' ' << record.second;
        ++count;
    };

    for (const auto &record : records)
    {
        if (!record.first.empty() && std::isalpha(static_cast<unsigned char>(record.first[0])))
        {
            writeRecord(record);
        }
    }

    currentCode.clear();
    for (const auto &record : records)
    {
        if (record.first.empty() || std::isalpha(static_cast<unsigned char>(record.first[0])))
        {
            continue;
        }
        writeRecord(record);
    }

    out << '\n';
    if (hzCount != nullptr)
    {
        *hzCount = count;
    }
    return true;
}

int txt2mb(char *txtPath, char *mbPath, int *HZcount)
{
    if (txtPath == nullptr || mbPath == nullptr || HZcount == nullptr)
    {
        FREEWB_DEBUG("param erro!");
        return 0;
    }

    MbDictionaryTable table;
    if (!parseTextDictionary(txtPath, table))
    {
        return 0;
    }

    if (!table.saveToFile(mbPath))
    {
        FREEWB_DEBUG("Cannot create target file!");
        return 0;
    }

    *HZcount = static_cast<int>(table.records().size());
    return 1;
}

int mb2txt(char *txtPath, char *mbPath, int *HZcount)
{
    if (txtPath == nullptr || mbPath == nullptr || HZcount == nullptr)
    {
        FREEWB_DEBUG("param erro!");
        return 0;
    }

    MbDictionaryTable table;
    if (!table.loadFromFile(mbPath))
    {
        FREEWB_DEBUG("Cannot open source file {}.", mbPath);
        FREEWB_DEBUG("码表格式错误");
        return 0;
    }

    if (!exportTextDictionary(txtPath, table, HZcount))
    {
        return 0;
    }

    return 1;
}

} // namespace freewb::tools
