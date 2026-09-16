#!/usr/bin/env python3
"""极点五笔格式皮肤 → freewb 自有皮肤格式转换器。

极点皮肤（[状态栏]/[候选窗] 中文键 + skin.bmp/candbutton.bmp 雪碧图）
转成 freewb 面板可识别的格式（[Skin]/[CandidateWin]/[ToolbarBg]/[ToolbarBtn] +
独立 PNG 切图）。

用法：
    python3 tools/convert_jidian_skins.py [resources/skin] [皮肤目录名...]

不带参数时转换 resources/skin 下所有极点格式皮肤。
原 skin.ini 备份为 skin.ini.jidian，雪碧图原文件保留。
"""

import configparser
import io
import os
import shutil
import sys

from PIL import Image

SECTION_BAR = "状态栏"
SECTION_CAND = "候选窗"

# 状态栏元素 → freewb 工具栏按钮
# (极点区域名, freewb 按钮前缀, 普通态图标名, 切换态图标名)
# 切换态从 skin_button.bmp 切，普通态从 skin.bmp / skinex.bmp 切
BAR_TOGGLE_BUTTONS = [
    ("全半角开", "btnCharWidth", "charwidth_full.png", "charwidth_half.png", "Full", "Half"),
    ("中英文开", "btnMark", "mark_cn.png", "mark_en.png", "Cn", "En"),
    ("GBK开", "btnCharSet", "charset_gbk.png", "charset_gb.png", "Gbk", "Gb"),
    ("繁简体开", "btnCharFont", "charfont_simp.png", "charfont_trad.png", "Simp", "Trad"),
]

BAR_SINGLE_BUTTONS = [
    ("屏幕查询", "btnSearch", "search.png"),
    ("造词", "btnGenerate", "generate.png"),
    ("软键盘开", "btnKeyboard", "keyboard.png"),
]

# 候选窗元素 → freewb [CandidateWin] 键
# (极点区域名, freewb 键, 输出文件名)
CAND_SINGLE_ICONS = [
    ("全角字符", "fullIco", "cand_full.png"),
    ("半角字符", "halfIco", "cand_half.png"),
    ("全角标点", "cnMarkIco", "cand_mark_cn.png"),
    ("半角标点", "enMarkIco", "cand_mark_en.png"),
]

# 翻页钮：freewb 的 0 态=不可翻(灰)，1 态=可翻；极点「…2」区域为灰态
CAND_PAGE_BUTTONS = [
    ("回到前页", "prev0PageIco", "prev1PageIco", "prev0.png", "prev1.png"),
    ("进到下页", "next0PageIco", "next1PageIco", "next0.png", "next1.png"),
]

IMG_EXTS = (".bmp", ".png")


def read_ini_text(path):
    raw = open(path, "rb").read()
    if raw.startswith(b"\xff\xfe") or raw.startswith(b"\xfe\xff"):
        text = raw.decode("utf-16", errors="replace")
    else:
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError:
            # GBK/GB18030；个别文件有坏字节（如 FlatLight），用 replace 兜底
            text = raw.decode("gb18030", errors="replace")
    # 丢弃含替换符的行：坏字节的畸形节头（如 "[鍊欓塢" 缺右括号）会让 configparser 报错
    return "\n".join(ln for ln in text.splitlines() if "\ufffd" not in ln)


def parse_ini(path):
    # 过滤极点皮肤里的 // 注释行，configparser 只认 ; 和 #
    lines = [ln for ln in read_ini_text(path).splitlines() if not ln.lstrip().startswith("//")]
    cp = configparser.ConfigParser(interpolation=None, strict=False)
    cp.optionxform = str
    cp.read_file(io.StringIO("\n".join(lines)))
    return cp


def get_int(cp, section, key, default=0):
    try:
        return int(cp.get(section, key).strip())
    except (configparser.Error, ValueError):
        return default


def get_rect(cp, section, name, suffix=""):
    """读取 <name>left/top/right/bottom（可加后缀 2），返回 (l, t, r, b)；无效返回 None。"""
    keys = [f"{name}left{suffix}", f"{name}top{suffix}", f"{name}right{suffix}", f"{name}bottom{suffix}"]
    if not all(cp.has_option(section, k) for k in keys):
        return None
    l, t, r, b = (get_int(cp, section, k) for k in keys)
    if r <= l or b <= t:
        return None
    return (l, t, r, b)


def find_file(folder, stem):
    """大小写不敏感地找 stem.bmp / stem.png。"""
    try:
        entries = os.listdir(folder)
    except OSError:
        return None
    lower_map = {e.lower(): e for e in entries}
    for ext in IMG_EXTS:
        hit = lower_map.get(stem.lower() + ext)
        if hit:
            return os.path.join(folder, hit)
    return None


def load_image(path):
    if path is None:
        return None
    try:
        return Image.open(path).convert("RGBA")
    except Exception:
        return None


def covers(img, rect):
    return img is not None and rect[2] <= img.width and rect[3] <= img.height


def crop_from(img, rect):
    return img.crop(rect) if covers(img, rect) else None


def slice_with_mask(img, rect, mask_pos):
    """从图集切图标；mask_pos 提供时按掩码亮度生成 alpha。"""
    icon = crop_from(img, rect)
    if icon is None:
        return None
    if mask_pos is not None:
        w, h = icon.size
        mask_rect = (mask_pos[0], mask_pos[1], mask_pos[0] + w, mask_pos[1] + h)
        if covers(img, mask_rect):
            icon.putalpha(img.crop(mask_rect).convert("L"))
    return icon


def save_png(img, path):
    if img is None:
        return False
    img.save(path)
    return True


def convert_skin(folder):
    ini_path = os.path.join(folder, "skin.ini")
    backup = ini_path + ".jidian"
    # 已转换过的皮肤从备份（极点原文）重新转换，保证可重复执行
    src_ini = backup if os.path.isfile(backup) else ini_path
    if not os.path.isfile(src_ini):
        return "跳过（无 skin.ini）"
    cp = parse_ini(src_ini)
    if not cp.has_section(SECTION_BAR):
        return "跳过（非极点格式）"

    toolbar_btn_lines = []

    skin_img = load_image(find_file(folder, "skin"))
    if skin_img is None:
        return "跳过（缺 skin 图）"
    skinex_img = load_image(find_file(folder, "skinex"))
    skinbtn_img = load_image(find_file(folder, "skin_button"))
    cand_img = load_image(find_file(folder, "candbutton"))
    back_img = load_image(find_file(folder, "back"))

    name = cp.get(SECTION_BAR, "皮肤名称", fallback="").strip() or os.path.basename(folder)

    # 普通态优先从 skin.bmp 切，区域超出（扩展行）时用 skinex.bmp
    def bar_slice(rect):
        return crop_from(skin_img, rect) or crop_from(skinex_img, rect)

    def bar_toggle_slice(rect):
        return crop_from(skinbtn_img, rect) or bar_slice(rect)

    # 背景与尺寸：收起态=skin，扩展态=skinex（无则同收起态）
    save_png(skin_img, os.path.join(folder, "bg_toolbar0.png"))
    size0 = skin_img.size
    if skinex_img is not None:
        save_png(skinex_img, os.path.join(folder, "bg_toolbar1.png"))
        size1 = skinex_img.size
    else:
        save_png(skin_img, os.path.join(folder, "bg_toolbar1.png"))
        size1 = size0

    # 扩展菜单钮（状态栏切换钮）：收起态图标取 skin，展开态取 skinex
    rect = get_rect(cp, SECTION_BAR, "状态栏切换钮")
    if rect:
        closed = crop_from(skin_img, rect) or crop_from(skinex_img, rect)
        opened = crop_from(skinex_img, rect) or closed
        if save_png(closed, os.path.join(folder, "extmenu_closed.png")) and opened:
            save_png(opened, os.path.join(folder, "extmenu_open.png"))
            toolbar_btn_lines += [
                "btnExtMenuFlg=1",
                f"btnExtMenuGeometry=@Rect({rect[0]} {rect[1]} {rect[2]-rect[0]} {rect[3]-rect[1]})",
                "btnExtMenuClosedImg=extmenu_closed.png",
                "btnExtMenuOpenImg=extmenu_open.png",
            ]
    if not any(l.startswith("btnExtMenuFlg") for l in toolbar_btn_lines):
        toolbar_btn_lines.append("btnExtMenuFlg=0")

    # 输入模式钮：极点「文本框」为静态名称图；各引擎态共用同一切图
    rect = get_rect(cp, SECTION_BAR, "文本框")
    if rect:
        icon = bar_slice(rect)
        if save_png(icon, os.path.join(folder, "mode.png")):
            w, h = rect[2] - rect[0], rect[3] - rect[1]
            toolbar_btn_lines += [
                "btnModeFlg=1",
                f"btnModeGeometry=@Rect({rect[0]} {rect[1]} {w} {h})",
                "btnModewbFontImg=mode.png",
                "btnModewbPyImg=mode.png",
                "btnModeStdPyImg=mode.png",
                "btnModeEnglishImg=mode.png",
                "btnModeCapsImg=mode.png",
            ]
    if not any(l.startswith("btnModeFlg") for l in toolbar_btn_lines):
        toolbar_btn_lines.append("btnModeFlg=0")

    for name_rect, prefix, normal_png, toggled_png, normal_key, toggled_key in BAR_TOGGLE_BUTTONS:
        rect = get_rect(cp, SECTION_BAR, name_rect)
        icon_n = bar_slice(rect) if rect else None
        icon_t = bar_toggle_slice(rect) if rect else None
        if save_png(icon_n, os.path.join(folder, normal_png)):
            save_png(icon_t, os.path.join(folder, toggled_png))
            w, h = rect[2] - rect[0], rect[3] - rect[1]
            toolbar_btn_lines += [
                f"{prefix}Flg=1",
                f"{prefix}Geometry=@Rect({rect[0]} {rect[1]} {w} {h})",
                f"{prefix}{normal_key}Img={normal_png}",
                f"{prefix}{toggled_key}Img={toggled_png if icon_t is not None else normal_png}",
            ]
        else:
            toolbar_btn_lines.append(f"{prefix}Flg=0")

    for name_rect, prefix, png in BAR_SINGLE_BUTTONS:
        rect = get_rect(cp, SECTION_BAR, name_rect)
        icon = bar_slice(rect) if rect else None
        if save_png(icon, os.path.join(folder, png)):
            w, h = rect[2] - rect[0], rect[3] - rect[1]
            toolbar_btn_lines += [
                f"{prefix}Flg=1",
                f"{prefix}Geometry=@Rect({rect[0]} {rect[1]} {w} {h})",
                f"{prefix}Img={png}",
            ]
        else:
            toolbar_btn_lines.append(f"{prefix}Flg=0")

    # 极点皮肤没有的按钮：设置 / LOGO
    toolbar_btn_lines.append("btnSettingFlg=0")
    toolbar_btn_lines.append("btnLogoFlg=0")

    # ---------- 候选窗 ----------
    cand_lines = []
    if back_img is not None and save_png(back_img, os.path.join(folder, "bg_center.png")):
        cand_lines.append("bgCenterImg=bg_center.png")

    if cand_img is not None:
        for name_rect, key, png in CAND_SINGLE_ICONS:
            rect = get_rect(cp, SECTION_CAND, name_rect)
            mask = None
            if rect:
                ml = get_int(cp, SECTION_CAND, f"{name_rect}_掩码left", -1)
                mt = get_int(cp, SECTION_CAND, f"{name_rect}_掩码top", -1)
                if ml >= 0 and mt >= 0:
                    mask = (ml, mt)
            icon = slice_with_mask(cand_img, rect, mask) if rect else None
            if save_png(icon, os.path.join(folder, png)):
                cand_lines.append(f"{key}={png}")

        for name_rect, key0, key1, png0, png1 in CAND_PAGE_BUTTONS:
            rect_on = get_rect(cp, SECTION_CAND, name_rect)       # 正常（可翻页）
            rect_off = get_rect(cp, SECTION_CAND, name_rect, "2")  # 灰态（不可翻页）
            mask_on = mask_off = None
            if rect_on:
                ml = get_int(cp, SECTION_CAND, f"{name_rect}_掩码left", -1)
                mt = get_int(cp, SECTION_CAND, f"{name_rect}_掩码top", -1)
                if ml >= 0 and mt >= 0:
                    mask_on = (ml, mt)
            if rect_off:
                ml = get_int(cp, SECTION_CAND, f"{name_rect}_掩码left2", -1)
                mt = get_int(cp, SECTION_CAND, f"{name_rect}_掩码top2", -1)
                if ml >= 0 and mt >= 0:
                    mask_off = (ml, mt)
            icon_on = slice_with_mask(cand_img, rect_on, mask_on) if rect_on else None
            icon_off = slice_with_mask(cand_img, rect_off, mask_off) if rect_off else None
            if save_png(icon_off or icon_on, os.path.join(folder, png0)):
                cand_lines.append(f"{key0}={png0}")
            if save_png(icon_on or icon_off, os.path.join(folder, png1)):
                cand_lines.append(f"{key1}={png1}")

    # 缺翻页图的皮肤回退到 default 皮肤的图标
    default_dir = os.path.join(os.path.dirname(os.path.abspath(folder)), "default")
    for key, png, fallback in (("prev0PageIco", "prev0.png", "prev0.png"),
                               ("prev1PageIco", "prev1.png", "prev1.png"),
                               ("next0PageIco", "next0.png", "next0.png"),
                               ("next1PageIco", "next1.png", "next1.png")):
        if not any(l.startswith(key) for l in cand_lines):
            src = os.path.join(default_dir, fallback)
            if os.path.isfile(src):
                shutil.copyfile(src, os.path.join(folder, png))
                cand_lines.append(f"{key}={png}")

    # ---------- 写 skin.ini ----------
    backup = ini_path + ".jidian"
    if not os.path.isfile(backup):
        shutil.copyfile(ini_path, backup)

    lines = ["[Skin]", f"name={name}", ""]
    lines += ["[CandidateWin]"] + cand_lines + [""]
    lines += ["[ToolbarBg]",
              "bgImg0=bg_toolbar0.png",
              "bgImg1=bg_toolbar1.png",
              f"size0=@Size({size0[0]} {size0[1]})",
              f"size1=@Size({size1[0]} {size1[1]})", ""]
    lines += ["[ToolbarBtn]"] + toolbar_btn_lines + [""]

    with open(ini_path, "w", encoding="utf-8", newline="\n") as fp:
        fp.write("\n".join(lines))

    n_btns = sum(1 for l in toolbar_btn_lines if l.endswith("Flg=1"))
    return f"完成：{name}（工具栏按钮 {n_btns} 个，候选窗图标 {len(cand_lines)} 项）"


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "resources", "skin")
    root = os.path.abspath(root)
    targets = sys.argv[2:] if len(sys.argv) > 2 else sorted(os.listdir(root))
    for entry in targets:
        folder = os.path.join(root, entry)
        if not os.path.isdir(folder):
            continue
        try:
            result = convert_skin(folder)
        except Exception as exc:  # 单个皮肤失败不影响其它
            result = f"失败：{exc}"
        print(f"[{entry}] {result}")


if __name__ == "__main__":
    main()
