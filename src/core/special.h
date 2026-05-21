#ifndef SPECIAL_H
#define SPECIAL_H

#include <ctime>
#include <string>
#include <vector>

#include "ifreewb.h"

namespace freewb
{

/** 时间模板（$y/$M）、金额/数字/日期格式化；用户词库与临时英文共用。 */
class Special final : public IFreewb
{
public:
    Special() = default;
    ~Special() override = default;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    /** 候选展示：展开 $ 时间占位；纯 ASCII 数字则做大写金额。 */
    void format(std::string &text) const;

    /** 是否含可展开的 $ 时间占位。 */
    bool hasTimePlaceholder(const std::string &text) const;

    /**
     * 临时英文正文（不含引导键）→ 最多 3 条候选。
     * 顺序与旧版 TableGetQuickCandWords 一致：金额 → 中文数字/日期 → 简写数字/日期。
     */
    std::vector<std::string> formatSpecialValues(const std::string &body) const;

private:
    /** 失败或无可格式化内容时返回空字符串。 */
    std::string formatMoney(const std::string &body) const;
    std::string formatNumber(const std::string &body) const;
    std::string formatNumberSimple(const std::string &body) const;
    /** flg: 0 中文日期，1 阿拉伯数字分段，2 yyyy-mm-dd */
    std::string formatDate(const std::string &body, int flg) const;

    static void expandTimePlaceholders(std::string &text, const std::tm &tm);
    static bool isAsciiDigitBody(const std::string &text);
    static bool isTimeSpec(char c);
    static bool scanTimePlaceholder(const std::string &text, std::size_t dollarPos, std::size_t &specPos);
    static std::string chineseYear(int year);
    static std::string chineseBelow100(int value, bool zeroWhenZero);
    static std::string expandTimeSpec(char spec, char next, const std::tm &tm, bool &consumedNext);

private:
    bool available_ = true;
};

} // namespace freewb

#endif
