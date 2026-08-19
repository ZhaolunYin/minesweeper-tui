# Minesweeper TUI
A TUI Minesweeper game written in C.

<img src="./assets/gameplay.gif" alt="Gameplay" height="500" width="auto">

## Installation
### Nix
If you have nix installed, you can do:
```bash
nix run github:zhaolunyin/minesweeper-tui
```

### Dependencies
The following dependencies must be installed **before** compilation:
- **make**
- **ncurses**

### Compilation
```bash
# Clone the repo
git clone https://github.com/ZhaolunYin/minesweeper-tui.git && cd minesweeper-tui

# build & run
make
./minesweeper

# or
make run
```

## Controls
### Keyboard

| Key   | Action |
|-------|--------|
| Enter | Covered square: Uncover square |
|       | Uncovered square: Chord (Uncover all unflagged cells around it when the correct amount of flags are around it.) |
| Arrow / Vim keys | Move cursor |
| F | Flag square |

### Mouse

| Button | Action |
|--------|--------|
| Left   | Covered square: Uncover square |
|        | Uncovered square: Chord (Uncover all unflagged cells around it when the correct amount of flags are around it.) |
| Right | Flag square |


