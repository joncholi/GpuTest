# PhysX + SDL3 GPU Test

This is a lightweight GPU-accelerated physics test application using NVIDIA PhysX 5.6.0 and SDL3.

The goal is to verify that GPU acceleration works correctly with PhysX and provides a simple 2D-like visualization of falling boxes with basic collision handling.

## Features

- PhysX 5.6.0 SDK with GPU support
- SDL3-based window and rendering
- Falling dynamic boxes with gravity
- Keyboard toggle for gravity (press `G`)
- On-screen GPU usage indicator
- Basic collision interaction between boxes

## Requirements

- Windows (x64)
- Visual Studio 2022
- CUDA Toolkit (12.x)
- CMake 3.20+
- SDL3 built or downloaded
- PhysX 5.6.0 SDK (cloned from [NVIDIA-Omniverse/PhysX](https://github.com/NVIDIA-Omniverse/PhysX))

## Setup Instructions

1. Clone the PhysX SDK and ensure GPU support is enabled:
    ```bash
    git clone --recursive https://github.com/NVIDIA-Omniverse/PhysX
    cd PhysX
    generate_projects.bat
    ```

2. Build the PhysX SDK using the generated Visual Studio solution.

3. Place this project under:
    ```
    PhysX-main/physx/compiler/GpuTest
    ```

4. Open the `PhysXSDK.sln` solution, and add or open the `GpuTest` project.

5. Ensure the following include paths and libraries are set in project settings:
    - Include paths:
      ```
      ../../source/physx/include
      ../../include
      ../../source/common/include
      ../../source/gpu/include
      path_to_sdl3_include
      ```
    - Link against:
      ```
      PhysX_64.lib
      PhysXCommon_64.lib
      PhysXExtensions_static_64.lib
      PhysXCudaContextManager_static_64.lib
      SDL3.lib
      ```

6. Copy required `.dll` files (PhysX and SDL3) next to the output `.exe`.

## Usage

- Run the program from Visual Studio or manually.
- Use **G** key to toggle gravity on/off.
- Boxes will fall, collide, and bounce around.
- A "GPU: ON" indicator is shown if GPU acceleration is active.

## Folder Structure

GpuTest/
├── main.cpp
├── CMakeLists.txt (optional)
├── README.md
└── resources/


## Credits

- Core code and integration support provided with the help of **ChatGPT (OpenAI)**, adapted to the user's PhysX SDK version and SDL3.
- Project built and maintained by the repository author.

## License

This project is a test harness for development purposes only. PhysX is licensed under NVIDIA's EULA. SDL is licensed under zlib/libpng license.