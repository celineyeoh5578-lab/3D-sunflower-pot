# 3D-sunflower-pot
This project creates a 3D simulation of a smart plant pot with various interactive elements. The scene includes a rotating plant pot, a sunflower with animated petals, a movable lid, a stopper mechanism, a lighting system, and detailed textures for various parts of the pot. The plant pot's behavior is customizable through keyboard inputs, allowing users to interact with the pot, its components, and the scene.  

Done by July 2025.  

## Features
1. Interactive Plant Pot
- Rotate the plant pot using the keyboard controls.
- Adjust the camera view (top, bottom, or default).
- Zoom in and out for closer inspection.

2. Lid Control
- Open and close the lid of the plant pot with key presses.
- Adjust the lid’s position smoothly.

3. Stopper Mechanism
- Remove or reset the stopper from the plant pot.

4. Lighting
- Toggle the light on or off to simulate different lighting conditions.

5. 3D Sunflower
- Rotating sunflower with animated petals created using Bézier curves.
- The sunflower petals change color to simulate different stages of bloom.

6. Textures
- Realistic textures for the pot, led arm, and floor.
- Supports texture switching using mouse clicks.

## Controls
1. Lid:
- o – Open Lid
- c – Close Lid
- r – Reset Lid

2. Stopper:
- w – Remove Stopper
- n – Reset Stopper

3. Rotation:
- s – Start Rotation
- p – Pause Rotation

4. Zoom:
- + – Zoom In
- - – Zoom Out

5. View:
- t – Top View
- b – Bottom View
- d – Default View

6. Light:
- l – Turn Light On
- k – Turn Light Off

## Project Structure
- **main.cpp**: The main C++ file that contains the code for initializing the OpenGL scene, rendering the 3D objects, and handling user inputs.
- **stb_image.h**: Used for loading textures into OpenGL.
