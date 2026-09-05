# OmaRPNCalc

An RPN calculator for [Omarchy](https://omarchy.org), styled after
[omacalc](https://github.com/omacom-io/omacalc) and built with Qt Quick.

The stack is unbounded (HP-48 style) with four levels visible. `enter` pushes
the typed number, or duplicates X when nothing is typed; typing after an
operation lifts the stack. Colours follow the current Omarchy theme and update
live when it changes; the window sizes itself to `omarchy display text size`
and re-flows live too (resizing the window scales the whole face; on
Hyprland the live resize goes through `hyprctl`, since the compositor ignores
a mapped floating window resizing itself).

## Build

```sh
bin/build        # -> build/omarpncalc
bin/test         # engine tests (Qt Test, offscreen)
bin/install      # build + install to /usr (sudo)
```

Or package it: `makepkg -si`.

Requires `qt6-base` and `qt6-declarative` (both part of Omarchy).

## Keys

| Key | Action |
|---|---|
| `0-9` `.` `e` | digits / decimal / exponent (`±` negates the exponent while typing it) |
| `Enter` | enter (push, or duplicate X) |
| `+` `-` `*` `/` | y+x, y−x, y×x, y÷x |
| `Backspace` | delete a digit, or drop X when nothing is typed |
| `Delete` / `d` | drop |
| `Tab` / `x` | swap |
| `r` / `R` | roll down / roll up |
| `s` | ± |
| `i` / `q` | 1/x, √x |
| `c` | c: discard the entry, else zero X; press again for ac (clear stack and memory) |
| `m` / `M` / `Ctrl+M` | mr (recall), m+, m− |
| `Ctrl+C` / `Ctrl+V` | copy X / paste as X |
| `Esc` | clear all; press again on an empty calculator to close |
| `Ctrl+W` | close |

## Omarchy integration

Bind it in `~/.config/hypr/bindings.lua`:

```lua
hl.unbind("SUPER + CTRL + Q")
hl.unbind("XF86Calculator")
o.bind("SUPER + CTRL + Q", "Calculator", "omarpncalc")
o.bind("XF86Calculator", "Calculator", "omarpncalc")
```

and float it in `~/.config/hypr/hyprland.lua`:

```lua
o.window("omarpncalc", { float = true })
o.window("omarpncalc", { center = true })
```
