#include "ConversionTool.h"

#include <cctype>
#include <cstring>
#include <fstream>
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

static bool parseRuleCell(const std::string &token, EngineRuleCell &cell)
{
    if (token.size() < 3)
    {
        return false;
    }

    switch (token[0])
    {
    case 'p':
    case 'P':
        cell.iFlag = 1;
        break;
    case 'n':
    case 'N':
        cell.iFlag = 0;
        break;
    default:
        return false;
    }

    cell.iWhich = static_cast<uint8_t>(token[1] - '0');
    cell.iIndex = static_cast<uint8_t>(token[2] - '0');
    return true;
}

static bool parseRuleExpression(const std::string &expr, EngineRuleBlock &block, uint32_t codeLength)
{
    block.cells.resize(codeLength);
    std::size_t start = 0;
    for (uint32_t i = 0; i < codeLength; ++i)
    {
        const std::size_t plus = expr.find('+', start);
        const std::string token = (plus == std::string::npos) ? expr.substr(start) : expr.substr(start, plus - start);
        if (!parseRuleCell(token, block.cells[i]))
        {
            return false;
        }
        if (plus == std::string::npos)
        {
            return i + 1U == codeLength;
        }
        start = plus + 1U;
    }
    return false;
}

static bool parseRuleLine(const std::string &line, EngineRuleBlock &block, uint32_t codeLength)
{
    const std::size_t eq = line.find('=');
    if (eq == std::string::npos || eq < 1)
    {
        return false;
    }

    if (startsWith(line, "三字词"))
    {
        block.iFlag = 0;
        block.iWords = 3;
        return parseRuleExpression(line.substr(eq + 1), block, codeLength);
    }

    if (eq < 2)
    {
        return false;
    }

    const char kind = line[0];
    if (kind == 'e' || kind == 'E')
    {
        block.iFlag = 0;
    }
    else if (kind == 'a' || kind == 'A')
    {
        block.iFlag = 1;
    }
    else
    {
        return false;
    }

    block.iWords = static_cast<uint8_t>(std::stoi(line.substr(1, eq - 1)));
    return parseRuleExpression(line.substr(eq + 1), block, codeLength);
}

static int ruleSlotFor(const EngineRuleBlock &block, uint32_t codeLength)
{
    if (block.iFlag == 0 && block.iWords == 2)
    {
        return 0;
    }
    if (block.iFlag == 0 && block.iWords == 3)
    {
        return 1;
    }
    if (block.iFlag == 1 && block.iWords == codeLength)
    {
        return 2;
    }
    return -1;
}

static std::string formatRuleLine(const EngineRuleBlock &block)
{
    if (block.cells.empty())
    {
        return {};
    }

    std::string key;
    if (block.iFlag == 0 && block.iWords == 3)
    {
        key = "三字词";
    }
    else if (block.iFlag == 0)
    {
        key = "e" + std::to_string(static_cast<int>(block.iWords));
    }
    else
    {
        key = "a" + std::to_string(static_cast<int>(block.iWords));
    }

    std::string expr;
    for (std::size_t i = 0; i < block.cells.size(); ++i)
    {
        if (i > 0U)
        {
            expr.push_back('+');
        }
        const EngineRuleCell &cell = block.cells[i];
        expr.push_back(cell.iFlag != 0 ? 'p' : 'n');
        expr.push_back(static_cast<char>('0' + cell.iWhich));
        expr.push_back(static_cast<char>('0' + cell.iIndex));
    }
    return key + "=" + expr;
}

static bool parseRuleSection(const std::vector<std::string> &lines, std::vector<EngineRuleBlock> &rules, uint32_t codeLength)
{
    static const char *const kDefaultRules[] = {"e2=p11+p12+p21+p22", "e3=p11+p21+p31+p32", "a4=p11+p21+p31+n11"};
    if (codeLength < 2U)
    {
        return false;
    }

    rules.assign(static_cast<size_t>(codeLength - 1U), EngineRuleBlock{});
    for (uint32_t i = 0; i < codeLength - 1U; ++i)
    {
        if (!parseRuleLine(kDefaultRules[i], rules[i], codeLength))
        {
            return false;
        }
    }

    bool found = false;
    for (const std::string &line : lines)
    {
        if (line.find('=') == std::string::npos || line.find_first_of("pPnN") == std::string::npos)
        {
            continue;
        }

        EngineRuleBlock block;
        if (!parseRuleLine(line, block, codeLength))
        {
            FREEWB_DEBUG("Invalid rule line: {}", line);
            return false;
        }

        const int slot = ruleSlotFor(block, codeLength);
        if (slot < 0)
        {
            FREEWB_DEBUG("Unsupported rule line: {}", line);
            return false;
        }

        rules[static_cast<size_t>(slot)] = block;
        found = true;
    }

    return found;
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
    bool inRuleSection = false;
    bool inDataSection = false;
    std::vector<std::string> ruleLines;
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
        if (inRuleSection)
        {
            if (startsWith(line, "[Text]"))
            {
                inDataSection = true;
                break;
            }
            ruleLines.push_back(line);
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
            inRuleSection = true;
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

    if (hasRule && !parseRuleSection(ruleLines, rules, 4U))
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
            if (text.empty())
            {
                continue;
            }
            if (!table.appendSortedRecord(code, text))
            {
                FREEWB_WARN("Source file is not sorted by code near: {}", code);
                return false;
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
    if (table.hasRule())
    {
        out << "[rule]\n";
        for (const EngineRuleBlock &block : table.phraseEncodeRules())
        {
            const std::string ruleLine = formatRuleLine(block);
            if (!ruleLine.empty())
            {
                out << ruleLine << '\n';
            }
        }
    }
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
