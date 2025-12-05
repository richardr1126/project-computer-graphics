# Final Project - CSCI5229 Computer Graphics

**Richard Roberson** - CSCI5229 Fall 2025

Implements a 3D archery simulation and environment renderer. Featuring a generated outdoor scene with dynamic lighting, day/night cycles, atmospheric effects, and interactive archery mechanics.

## What remains to be done
- Sweep test (ray cast) for arrow collision/impact detection. Arrows should get stuck in targets and stop moving. Different areas of the target will yield different scores.
- Allow 15 shot arrows until the game is over. Keep user high score on disk and display it in the HUD.

## Highlighted Grad-level Features
- **Normal-mapped rock mountain ring using GLSL shaders (color + normal map, fog-aware, togglable with `B`).**
- Algorithmic tree generation with branching structure and alpha-blended leaves.
- Quality/performance optimizations highlighted above.
- Sweep test (ray casting) for arrow collision detection.

## All Features Implemented
- **Objects**:
  - **Trees**:
    - Procedurally generated trees with branching structure.
    - Textured bark and leaves with alpha blending.
  - **Terrain**:
    - Forest ground with height variations and normals using display lists.
    - Mountain ring surrounding the scene with noise-based height variations.
    - Normal-mapped rock texture on the distant mountain ring (color + normal map in a GLSL shader, togglable at runtime).
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
    - **Normals Debugging**: Toggle display of normals for all objects.

- **Archery Mechanics**:
  - **Shooting**: First-person shooting with charge-up mechanic. Hold right-click to charge power (visualized by dynamic crosshair), release to shoot.
  - **Physics**: Arrows follow physics trajectories with gravity.

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
  - **Ground Display List + Strips**: The terrain mesh is precomputed once (heights + normals) and cached in an OpenGL display list rendered as row-wise `GL_TRIANGLE_STRIP`s.

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
zip -r final.zip . -x ".git/*"
```

## Code Reuse and AI

Many pieces of code from previous assignments in CSCI5229 (written by Willem A. (Vlakkies) Schreuder) were reused and adapted for this project, including the base makefile and C environment setup. I have tried to clearly label reused portions and functions with the original author's name in comments (i.e., "Original Author: Willem A. (Vlakkies) Schreuder") wherever possible, but it is possible that I may have missed labeling some reused code due to the evolution of the code over time.

AI was used to help in various areas, which primarily consisted of areas where I used AI in tandem with my own code and original ideas to produce the final result; the primary way I tend to use AI. In these cases, I have labeled the relevant areas "helped by AI". In other areas, I used AI to generate entire helper functions and these are labeled with comments "generated by AI". And lastly, I also used AI to add or improve some of the comments in the code as well as updating the README when controls in the HUD changed.

---

## Key Bindings

### View Controls
| Key    | Action |
|--------|--------|
| TAB    | Toggle view modes: Perspective (orbit) ↔ First-Person |
| +/-    | Change field of view (perspective modes) |
| [/ ]   | Zoom in/out (orbit modes only) |
| 0      | Reset view (camera position/angles, FOV) |
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
| n/N    | Toggle normals debug lines |
| o/O    | Toggle texture filtering optimizations (mipmaps + anisotropic filtering) |
| f/F    | Toggle distance fog on/off |
| b/B    | Toggle normal-mapped rock mountains (GLSL normal map on distant ring) |

## Texture credits

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06/22325917510">free seamless texture autumn leaves 2</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06">zaphad1</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/44071822@N08/4727355663">High Quality Tileable Light Wood Texture 1</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/44071822@N08">webtreats</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06/8494824780">Soft Fur</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06">Filter Forge</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06/22030135762">bark 4</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06">zaphad1</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

### PBR Textures

Created using Rock 062 from ambientCG.com, licensed under the Creative Commons CC0 1.0 Universal License.
