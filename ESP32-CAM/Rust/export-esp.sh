# This is a translation of C:\Users\%USER%\export-esp.ps1 in BASH
# To use this:
#   . export-esp.sh
# to source into current bash env

export LIBCLANG_PATH="C:\Users\$USER\.rustup\toolchains\esp\xtensa-esp32-elf-clang\esp-clang\bin\libclang.dll"
export PATH="${PATH}:"$(cygpath "C:\Users\$USER\.rustup\toolchains\esp\xtensa-esp32-elf-clang\esp-clang\bin")
export PATH="${PATH}:"$(cygpath "C:\Users\$USER\.rustup\toolchains\esp\xtensa-esp-elf\bin")
