#

In `Cargo.toml`:
```toml
[[package.metadata.esp-idf-sys.extra_components]]
component_dirs = "components/esp32-camera"
bindings_header = "components/bindings.h"
bindings_module = "camera"
```

And:
```shell
$ cargo add esp-idf-sys
$ mkdir components
$ (pushd components ; git clone https://github.com/espressif/esp32-camera.git )
```

Note: that's just for quick testing. It should be a git-submodule later.

Build and flash+run:
```shell
$ time cargo build -vv
$ cargo run
```

