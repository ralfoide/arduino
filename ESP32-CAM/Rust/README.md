# Rust on the ESP32-CAM

## Toolchain

Cygwin:

```
$ rustup -V
    Rustup ==> 1.28.2
    Rustc  ==> 1.86.0
    Active toolchain: 1.86-x86_64-pc-windows-msvc

    
$ cargo install espup --locked

$ espup install --default-host x86_64-pc-windows-msvc --targets esp32,esp32s2
```

I only have the Xtensa-based ESP32s so I only need targets `esp32` and `esp32s2`.


## Setup Notes

`espup` creates a C:\Users\%USER%\export-esp.ps1 file that must be sources to add tools to the PATH.

Since I work in Cygwin/MSYS2, there's a conversion of that in `/export-esp.sh`.

To use this, source the variables in the current bash environment:
```shell
. export-esp.sh
```

## Template projects

