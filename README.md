## How to build

```
meson build
ninja -C build
```
Layer-shell support will be enabled automatically if [gtk-layer-shell](https://github.com/wmww/gtk-layer-shell) development files are installed.

## How to build without gtk-layer-shell

```
meson build -Dlayershell=disabled
ninja -C build
```
