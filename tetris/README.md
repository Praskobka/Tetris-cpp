# Tetris SFML 3

A complete, playable Tetris implementation built with C++17 and SFML 3.0.2.

## Project Overview
This project is a graphical version of the classic puzzle game Tetris. It features a clean interface, progressive difficulty, and persistent high scores. The game is designed to be portable and easy to build using CMake and vcpkg.

## Features
- **Classic Gameplay**: All 7 standard tetrominoes with correct shapes and colors.
- **Ghost Piece**: Visual projection showing exactly where the piece will land.
- **Persistence**: High scores are saved to the user's `%APPDATA%` folder.
- **Dynamic Audio**: Programmatically generated sound effects for movements, rotations, line clears, and game over.
- **Danger Warning**: A red horizontal line near the top of the board indicates a critical game state.
- **Progressive Difficulty**: Game speed increases as the player clears lines and levels up.

## Keyboard Controls
| Key | Action |
|---|---|
| **A** / **Left Arrow** | Move Left |
| **D** / **Right Arrow** | Move Right |
| **S** / **Down Arrow** | Soft Drop |
| **W** / **Up Arrow** | Rotate Piece |
| **Space** | Hard Drop |
| **P** / **Escape** | Pause Game |
| **R** | Restart Game |

## Technologies Used
- **Language**: C++17
- **Framework**: SFML 3.0.2 (Graphics, Window, System, Audio)
- **Build System**: CMake
- **Dependency Management**: vcpkg
- **Compiler**: MSVC (Visual Studio 2022+)

## How to Run the Precompiled Release
1. Download the `TetrisGame-Windows-x64.zip` archive from the Releases section.
2. Extract the archive to any folder on your Windows PC.
3. Open the extracted `TetrisGame_Final` folder.
4. Run `tetris.exe`.

*Note: If the game fails to launch, ensure you have the [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe) installed.*

## How to Build from Source
### Prerequisites
- Install [vcpkg](https://github.com/microsoft/vcpkg).
- Install SFML 3.0.2:
  ```bash
  vcpkg install sfml:x64-windows
  ```

### Build Steps (Visual Studio)
1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```
2. Configure the project with CMake:
   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
   ```
3. Build the executable:
   ```bash
   cmake --build . --config Release
   ```
