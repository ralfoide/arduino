# Rust on the ESP32-CAM

## Toolchain

Cygwin:

```shell
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

### For `no_std` projects

```shell
$ esp-generate --headless --chip esp32 -o unstable-hal -o alloc -o wifi -o log -o esp-backtrace -o vscode -O . esp-rs-no_std
$ cargo build
$ cargo run
```

`cargo run` builds and flashes to connected ESP32 using espflash.
The command to do is is in `.cargo/config.toml`.

### For `std` projects

For `std` projects, the `esp-generate` tool was useless.
Instead I use the git (esp-idf-template)[https://github.com/esp-rs/esp-idf-template which I’ve used in the past:


```shell
$ cargo generate esp-rs/esp-idf-template cargo
$ rm <project-folder>/.git -rvI
```

Specify the project name/forlder when requested, target is `esp32`, and use the latest `ESP-IDF 5.3.3`.

**Warning**: This generates a config that does not work out of the box on Windows / Cygwin.

The main issue is path lengths. Building the `esp-idf-sys` crate is the problem as this is an _entire_ checkout of the
Espressif ESP-IDF git repo and all its associated tools (including its own Python 3.10.9 and an entire gcc toolchain!).
The build paths quickly override the 250 character limit, and there are built-in checks to prevent it from failing.

It will generate errors in the style of "Error: Too long output directory ... Shorten your project path down to no more than 10 characters".

However these are easy to solve and work around:

```shell
$ vim .cargo/config.tom
[env]
ESP_IDF_PATH_ISSUES = "warn"
ESP_IDF_TOOLS_INSTALL_DIR = { value = "global" }
```

These (ESP-IDF Build Options)[https://github.com/esp-rs/esp-idf-sys/blob/master/BUILD-OPTIONS.md] do two things:
  - `ESP_IDF_PATH_ISSUES` transforms the 250-char limit error checks in mere warnings.
  - `ESP_IDF_TOOLS_INSTALL_DIR` changes the location of the `.embuild` from the current project path (a.k.a. "workspace")
    to the "global" location, namely `C:\Users\%USER%\esp-idf`.

This is enough to prevent the long path errors.

Building the `esp-idf-sys` create takes about 10-15 minutes on my system and uses about 3.8 GB _per version of the IDF_ in that global `esp-idf` folder.
It also seems to get rebuilt when the `.cargo/config.toml` changes.
To make the long build less opaque, build using the very verbose flag:

```shell
$ cargo build -vv
```


~~
