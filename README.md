# Final Project - CSCI5229 Computer Graphics

**Richard Roberson** - CSCI5229 Fall 2025

Implements a 3D archery simulation and environment renderer. Featuring a generated outdoor scene with dynamic lighting, day/night cycles, atmospheric effects, and interactive archery mechanics.

> **Note:** After seeing your comments on the project review I decided to forgoe the addition of another animated animal object in favor of doing normal-mapped terrain shaders for both the forest ground and rocky mountains. I wanted to make sure my project had a solid graduate-level feature, and I felt that the terrain shader addition added more value to the project than another animated object would have. I hope this is acceptable.

## Highlighted Grad-level Features
- **Normal-mapped terrain shader (forest ground + mountain rock ring) using GLSL shaders (color + normal maps, fog-aware, togglable with `B`).**
- Sweep test (ray casting) for arrow collision detection. With animated sticky arrow behavior.
- Algorithmic tree generation with branching structure and alpha-blended leaves.
- Dynamic sky and distance-based atmospheric fog that follows the time of day.
- Quality/performance optimizations highlighted below.

## All Features Implemented
- **Objects**:
  - **Trees**:
    - Procedurally generated trees with branching structure.
    - Textured bark and leaves with alpha blending.
  - **Terrain**:
    - Forest ground with height variations and normals using display lists.
    - Mountain rock ring surrounding the scene with noise-based height variations.
    - **Normal-mapped terrain shader:** forest ground and mountain rock ring both use color + normal maps with a shared terrain shader, fog-aware and togglable with `B`.
  - **Bullseyes**: Three textured bullseye targets with animated motion.
  - **Arrow**: Physics-based projectile that can be shot from the camera position.
  - **Light Sphere**: Smooth light source for the scene, which transitions between sun and moon lighting.

- **Environment**:
  - **Day/Night Cycle**: Dynamic sky with smooth transitions between day (blue gradient) and night (dark blue gradient). The time of day is determined by the rotation degree of the light source, 4 rotations is a full day/night cycle, with lighting intensity adjusting accordingly.
    - **World Cycle Controls**: Pause/resume the environment time cycle, manually step through time, adjust cycle speed (affects light rotation speed), and adjust light height and distance from a single control group.
  - **Atmospheric Fog**: Distance-based fog that blends object color with the sky color based on distance from the camera.
    - Fog color smoothly follows the day/night cycle (blue-tinted in day, darker at night).
    - Fog is stronger/denser at night, and very subtle during the day so nearby terrain remains clear.
  - **Lighting**: Animated light source with ambient, diffuse, and specular components using smooth shading. Shift between moon-like components and sun-like components based on the time of day.

- **Archery Mechanics**:
  - **Shooting**: First-person shooting with charge-up mechanic. Hold right-click to charge power (visualized by dynamic crosshair), release to shoot.
  - **Physics**: Arrows follow physics trajectories with gravity.
  - **Collision**: Arrows stick to targets using ray-cast detection.
  - **Scoring**: Points awarded based on accuracy and target difficulty (smaller targets with fewer rings award more points). High score is saved to disk.
  - **Game Loop**: Limited to 15 arrows per round. Game Over status is displayed in the HUD.

- **View Modes**: Switch between perspective (orbit) and first-person views.
- **Smooth First-Person Move & Look**: Hold WASD to move, click-drag mouse to look around; motion is frame-rate independent, diagonals normalized, and camera angles use smooth double precision.
- **BMP Alpha-channel Loading**: LoadTexBMP() now supports 32-bit BMPs with alpha channels.

## Quality/Performance Optimizations

- **Trees & Leaves**:
  - **Two-pass trees**: Trees are drawn in two passes: an opaque pass for trunks and branches, then a transparent pass for alpha-blended leaves. The leaf pass walks the same tree structure but only draws leaf billboards, so bark geometry is not redrawn.
  - **Shared forest layout**: Both passes share the same helper that arranges trees in rings around the scene, using the same random seeds. That keeps tree positions, shapes, and leaf placement identical between passes.
  - **Bark culling**: During the bark pass, back-face culling is enabled and `glFrontFace` is set to clockwise to match the tree mesh winding, then restored. This skips work on the hidden back sides of trunks and branches without affecting leaf rendering.

- **Terrain & Ground**:
  - **Culling for Terrain**: The ground and mountain meshes have back-face culling enabled, reducing fragment processing on downward-facing triangles.
  - **Display List + Strips**: Both the terrain meshes are precomputed once (heights + normals) and cached in an OpenGL display list rendered as row-wise `GL_TRIANGLE_STRIP`s.
  - **Normal-mapped terrain shader**: The terrain shader combines color and normal maps, applies fog based on distance, and is optimized to minimize calculations in the fragment shader.

- **Rendering & GL State**:
  - **Reduced State Churn**: Leaf texture is bound once for the entire transparent pass; per-leaf `glEnable(GL_TEXTURE_2D)`/`glBindTexture` calls were removed. Per-frustum texture parameter changes were removed from hot loops.
  - **Disabled GL_NORMALIZE**: Normals are pre-normalized for trunks/ground, and lighting is off for the light sphere’s scale. Disabling `GL_NORMALIZE` removes per-vertex renormalization overhead.
  - **Swap-Only Present**: Removed an explicit `glFlush()` before buffer swap; rely on `glutSwapBuffers()` which flushes implicitly, reducing driver overhead slightly.

- **Texture Quality & Tuning**:
  - **Anisotropic Filtering (if available)**: Texture loader enables the maximum supported anisotropy via `GL_EXT_texture_filter_anisotropic` for sharper textures at grazing angles.
  - **Texture Filtering Toggle**: Press `o/O` to switch between optimized filtering (mipmaps + anisotropic filtering when supported) and a basic linear mode for comparison; the HUD reports the current state.

## Run the program

```
make        # builds the project
./final   # launch the full scene
```

## Zip File Contents
```bash
zip -r final.zip . -x ".git/*" "highscore.txt" ".gitignore"
```

## Estimated Time to Completion

~70 hours

## Code Reuse and AI

Many pieces of code from previous assignments in CSCI5229 (written by Willem A. (Vlakkies) Schreuder) were reused and adapted for this project, including the base makefile and C environment setup. I have tried to clearly label reused portions and functions with the original author's name in comments (i.e., "Original Author: Willem A. (Vlakkies) Schreuder") wherever possible.

AI was used to help in various areas, which primarily consisted of areas where I used AI in tandem with my own code and original ideas to produce the final result; the primary way I tend to use AI. In these cases, I have labeled the relevant areas "helped by AI". In other areas, I used AI to generate entire helper functions and these are labeled with comments "generated by AI". And lastly, I also used AI to add or improve some of the comments in the code as well as updating the README when controls in the HUD changed.

---

## Key Bindings

### View Controls
| Key    | Action |
|--------|--------|
| TAB    | Toggle view modes: Perspective (orbit) ↔ First-Person |
| +/-    | Change field of view (perspective modes) |
| [/ ]   | Zoom in/out (orbit modes only) |
| 0      | Reset view and restart game (score/arrows) |
| h/H    | Cycle HUD modes (0=hint only, 1=controls, 2=all) |
| ESC    | Exit |

### Camera & Interaction Controls
| Key    | Action |
|--------|--------|
| Left-click drag | Look around in first-person (click and drag left mouse to yaw/pitch; works while moving) |
| Right-click | Hold to charge shot (crosshair turns red), release to fire arrow |
| arrows | Look around in perspective (orbit) mode |
| w/s    | Move forward/backward (first-person mode only) |
| a/d    | Strafe left/right (first-person mode only) |

### World Cycle Controls (Lighting + Sky/Time)
| Key    | Action |
|--------|--------|
| l/L    | Toggle lighting on/off |
| 1/2    | Raise/lower light height |
| 3/4    | Increase/decrease light distance |
| 5      | Pause/resume day/night cycle and light rotation |
| 6/7    | Increase/decrease cycle speed (affects both time and light rotation) |
| 9      | Manual time step forward (when paused) |

### Special Controls
| Key    | Action |
|--------|--------|
| o/O    | Toggle texture filtering optimizations (mipmaps + anisotropic filtering) |
| f/F    | Toggle distance fog on/off |
| b/B    | Toggle normal-mapped terrain (forest ground + mountain rock ring) |

## Texture credits

- **High Quality Tileable Light Wood Texture 1**  
  Source: [webtreats on Flickr](https://www.flickr.com/photos/44071822@N08/4727355663)  
  License: [CC BY 2.0](https://creativecommons.org/licenses/by/2.0/?ref=openverse)

- **Bark 4**  
  Source: [zaphad1 on Flickr](https://www.flickr.com/photos/25797459@N06/22030135762)  
  License: [CC BY 2.0](https://creativecommons.org/licenses/by/2.0/?ref=openverse)

- **Rock 062**  
  Source: [ambientCG.com](https://ambientcg.com/a/Rock062)  
  License: [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)

- **Ground 037**  
  Source: [ambientCG.com](https://ambientcg.com/a/Ground037)  
  License: [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
