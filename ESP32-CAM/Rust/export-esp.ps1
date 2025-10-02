$Env:LIBCLANG_PATH = "C:\Users\$Env:USERNAME\.rustup\toolchains\esp\xtensa-esp32-elf-clang\esp-clang\bin\libclang.dll"

$Env:PATH = "C:\Users\$Env:USERNAME\AppData\Local\Programs\Python\Python311;"+$Env:PATH

$Env:PATH = "C:\Users\$Env:USERNAME\.rustup\toolchains\esp\xtensa-esp32-elf-clang\esp-clang\bin;" + $Env:PATH
$Env:PATH = "C:\Users\$Env:USERNAME\.rustup\toolchains\esp\xtensa-esp-elf\bin;" + $Env:PATH

echo $Env:PATH
