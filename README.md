# OmaRPNCalc

An RPN calculator for [Omarchy](https://omarchy.org), styled after
[omacalc](https://github.com/omacom-io/omacalc) and built with Qt Quick.

The stack is unbounded (HP-48 style) with four levels visible. `enter` pushes
the typed number, or duplicates X when nothing is typed; typing after an
operation lifts the stack. Colours follow the current Omarchy theme and update
live when it changes.

The window is a fixed 400x660, shrinking only for a screen too small to hold
that. It does not follow `omarchy display text size`: font sizes come from the
window size, so following that setting meant growing the whole window, which
made a calculator you summon and dismiss take over the screen. Resize it by
hand (`Super+=` / `Super+-` on Hyprland) and the whole face scales with it.

## Build

```sh
bin/build        # -> build/omarpncalc
bin/test         # engine tests, then a QML self-test (both offscreen)
bin/install      # build + install to /usr (sudo)
```

Or package it: `makepkg -si`.

`omarpncalc --self-test` loads the interface, spins the event loop briefly and
exits non-zero if QML reported anything. The engine tests never load the
interface and the compiler never sees it, so this is what catches a QML slip.

Requires `qt6-base` and `qt6-declarative` (both part of Omarchy).

## Keys

| Key | Action |
|---|---|
| `0-9` `.` `,` `e` | digits / decimal / exponent (`±` negates the exponent while typing it) |
| `Enter` / `=` | enter (push, or duplicate X) |
| `+` `-` `*` `/` | y+x, y−x, y×x, y÷x |
| `Backspace` | delete a digit, or drop X when nothing is typed |
| `Delete` / `d` | drop |
| `Tab` / `x` | swap |
| `r` / `R` | roll down / roll up |
| `s` | ± |
| `i` / `q` | 1/x, √x |
| `c` | c: discard the entry, else zero X; press again for ac (clear stack and memory) |
| `m` / `M` / `Ctrl+M` | mr (recall), m+, m− |
| `Ctrl+C` / `Ctrl+V` | copy X / paste as X (`Super` works in place of `Ctrl`) |
| `Esc` | clear all; press again on an empty calculator to close |
| `Ctrl+W` / `Ctrl+Q` | close |

Copying takes the value rather than the keystrokes, so what reaches the
clipboard always pastes back. Pasting accepts thousands separators, both
`1,234.5` and `1 234.5`.

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
