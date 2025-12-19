# Toy Airplane Shop - OpenGL Project

## Project Structure
- **main.cpp**: Main application source code.
- **InitShader.cpp**: Shader initialization helper.
- **Angel.h**: Standard header file.
- **vec.h, mat.h**: Mathematical helper libraries.
- **CheckError.h**: Debugging utility.
- **vshader.glsl**: Vertex Shader.
- **fshader.glsl**: Fragment Shader.

## Compilation Instructions (Linux)
Ensure you have `freeglut3-dev`, `libglew-dev`, and `mesa-common-dev` installed.

```bash
g++ main.cpp -o toy_shop -lglut -lGLEW -lGL -lGLU
./toy_shop
```

## Compilation Instructions (Visual Studio)
1. Create a new "Empty Project" in Visual Studio.
2. Add `main.cpp` and `InitShader.cpp` to Source Files.
3. Add `Angel.h`, `vec.h`, `mat.h`, `CheckError.h` to Header Files.
4. Ensure `freeglut` and `glew` are properly linked via NuGet or local includes.
5. Place `vshader.glsl` and `fshader.glsl` in the same directory as the executable (or Project directory).

## Controls
- **0**: Camera Control Mode (WASD to move, Q/E Up/Down).
- **1-8**: Select specific aircraft to control.
    - **1**: Jet
    - **2**: Propeller Plane
    - **3**: Helicopter
    - **4**: Paper Plane
    - **5**: Drone
    - **6**: Rocket
    - **7**: Balloon
    - **8**: Fighter Jet
    - **Controls**: WASD to move, QE to lift, RF to pitch, UJ/IK for specific parts.
- **9**: Toggle Lights.
- **ESC**: Exit.