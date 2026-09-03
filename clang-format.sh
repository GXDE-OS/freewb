#!/usr/bin/env bash
# 使用项目根目录下的 .clang-format 配置，原地格式化 .c / .h / .cpp 源文件。
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

# 优先使用能解析本仓库 .clang-format 的版本（含 SeparateDefinitionBlocks 等需 LLVM 15+）
pick_clang_format() {
    local c
    for c in clang-format-18 clang-format-17 clang-format-16 clang-format-15 clang-format; do
        command -v "$c" >/dev/null 2>&1 || continue
        if echo 'int x;' | "$c" --style=file -assume-filename=t.cpp >/dev/null 2>&1; then
            printf '%s\n' "$c"
            return 0
        fi
    done
    return 1
}

CLANG_FORMAT="$(pick_clang_format)" || {
    echo "错误: 未找到可用的 clang-format，或当前版本无法解析 .clang-format。" >&2
    echo "请安装 LLVM 15+ 的 clang-format（例如: sudo apt install clang-format-15）。" >&2
    exit 1
}

# 排除版本库、常见构建输出目录及第三方源码，避免误处理生成文件
find . -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' \) \
    ! -path './.git/*' \
    ! -path './build/*' \
    ! -path './cmake-build-*/*' \
    ! -path './out/*' \
    ! -name 'sqlite3.c' \
    ! -name 'sqlite3.h' \
    ! -name 'SimpleIni.h' \
    ! -name 'ConvertUTF.c' \
    ! -name 'ConvertUTF.h' \
    -print0 |
    xargs -0 "$CLANG_FORMAT" -i --style=file

echo "已完成：已使用 ${CLANG_FORMAT} 按 .clang-format 格式化工程中的 .c / .h / .cpp 文件。"
