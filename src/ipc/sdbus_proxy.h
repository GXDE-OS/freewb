#ifndef SDBUS_PROXY_H
#define SDBUS_PROXY_H

#include <string>
#include <vector>

#include <systemd/sd-bus.h>

#include "dbus.h"
#include "freewb.h"

namespace freewb::ipc
{

/** libsystemd sd-bus 会话总线上的 IDBus，并实现 freewb::IFreewb。 */
class SDBusProxy final : public IDBus, public ::freewb::IFreewb
{
public:
    explicit SDBusProxy(void *sd_event_handle = nullptr, int priority = 0);
    ~SDBusProxy() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    bool bindDBusSignalCallback(DBusSignalCallback callback) override;

    /** @brief 更新属性。 */
    /** @param payload 属性。 */
    void emitUpdateProperties(const ToolbarPropertiesPayload &payload) override;

    /** @brief 显示工具栏。 */    
    void emitShowToolbar() override;
    
    /** @brief 隐藏工具栏。 */
    void emitHideToolbar() override;

    /** @brief 更新候选框位置
     * @param payload 候选框位置。
     */
    void emitUpdateSpotRect(const SpotRectPayload &payload) override;
    
    /** @brief 更新候选框
     * @param payload 候选框。
     */
    void emitUpdateCandidate(const CandidatePayload &payload) override;

    /** @brief 更新候选框文本
     * @param payload 候选框文本。
     */
    void emitUpdatePreeditText(const CandidatePreeditPayload &payload) override;
    
    /** @brief 更新候选框光标位置
     * @param caret 光标位置。
     */
    void emitUpdatePreeditCaret(int caret) override;
    
    /** @brief 更新候选框辅助文本
     * @param payload 候选框辅助文本。
     */
    void emitUpdateAux(const CandidateAuxPayload &payload) override;

public: // settings 通过 D-Bus 调用 freewb-settings 服务的方法
    /**
     * @brief 添加用户词组
     * @param flg 操作标志：0-显示待造词提示，1-确认造词，2-取消造词，3-自定义词组编码。
     * @param wordCode 词组编码。
     * @param wordText 词组文本。
     */
    void callAddUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText);

    /**
     * @brief 删除用户词组
     * @param flg 操作标志：0-显示待删词提示，1-确认删词，2-取消删词。
     * @param wordCode 词组编码。
     * @param wordText 词组文本。
     */
    void callDeleteUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText);

    /**
     * @brief 查询词典
     * @param wordText 查询文本（词条）。
     */
    void callDictQueryMethod(const std::string &wordText);
    
    /**
     * @brief 切换输入模式
     * @param inputMode 输入模式：0-五笔字型，1-五笔拼音，2-拼音输入。
     */
    void callSwitchInputModeMethod(int inputMode);
    
    /** @brief 切换皮肤。 */
    void callSwitchSkinMethod();
    
    /**
     * @brief 启用/禁用标点自动配对
     * @param flg 0-关闭，1-开启。
     */
    void callSwitchSmartPuncMethod();
    
    /** @brief 切换字符集（GB/GBK）。 */
    void callSwitchCharSetMethod();
    
    /**
     * @brief 启用/禁用重码上屏校对
     * @param flg 0-关闭，1-开启。
     */
    void callSwitchRecodeProofMethod();
    
    /**
     * @brief 切换词组常用/非常用状态。
     * @param wordText 词组文本。
     * @param flg 0-常用，1-非常用。
     */
    void callSwitchUncommonParseStateMethod(const std::string &wordText, int flg);
    
    /**
     * @brief 切换简繁体输出模式。
     * @param flg 0-简体，1-繁体。
     */
    void callSwitchChttransMethod();
    
    /** @brief 打开界面设置。 */
    void callOpenUiSettingMethod();
    
    /** @brief 显示版本信息。 */
    void callShowVersionInfoMethod();
    
    /** @brief 打开专业设置。 */
    void callOpenProfessionalSettingMethod();
    
    /** @brief 编辑快捷码表。 */
    void callModQuickTableMethod();
    
    /** @brief 编辑用户码表。 */
    void callModUserTableMethod();
    
    /** @brief 编辑五笔码表。 */
    void callModWubiTableMethod();
    
    /** @brief 编辑拼音码表。 */
    void callModPinyinTableMethod();
    
    /** @brief 打开极点目录。 */
    void callOpenConfDirMethod();
    
    /**
     * @brief 切换虚拟键盘模式。
     * @param flg 切换方向：0-向后切换，1-向前切换。
     */
    void callSwitchVirtualKeyboardModeMethod(int flg);
    
    /** @brief 关闭虚拟键盘。 */
    void callCloseVkBoardMethod();
    
    /** @brief 切换词库。 */
    void callSwitchTableMethod();
    
    /**
     * @brief 切换全角半角
     * @param flg 0-不切换字符宽度，非0-切换一次字符宽度（全角/半角）。
     */
    void callSwitchCharWidthModeMethod();

    /**
     * @brief 切换中英文标点
     * @param flg 0-不切换标点模式，非0-切换一次标点模式（中/英文标点）。
     */
     void callSwitchPuncModeMethod();
    
    /**
     * @brief 获取系统剪贴板内容。
     * @return 剪贴板文本。
     */
    std::string callGetClipboardMethod();
    
    /** @brief 切换大小写状态。 */
    void callToggleCapsStateMethod();

private:
    static std::string toolbarPayloadToPropertyLine(const ToolbarPropertiesPayload &p);

    void emitRegisterPropertiesSignal(const std::vector<std::string> &props);
    void emitImeSignal(const char *member, const char *types, ...) const;
    void sendPanelMethod(const char *member, const char *types, ...) const;
    void callSettingsMethod(const char *member, const char *types, ...) const;
    std::string callSettingsMethodReplyString(const char *member) const;

    static int handlePanelSignal(sd_bus_message *m, void *userdata, sd_bus_error *retError);

    bool registerPanelMatches();
    bool registerInputMethodObject();
    void clearSlots();
    void closeBus();

private:
    sd_bus *bus_ = nullptr;
    DBusSignalCallback onDBusSignal_ = nullptr;
    sd_bus_slot *panelSignalSlot_ = nullptr;
    bool available_ = true;
};

} // namespace freewb::ipc

#endif
