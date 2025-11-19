# Final Project - CSCI5229 Computer Graphics

**Richard Roberson**
CSCI5229 Fall 2025

Implements texture mapping on a 3D scene with lighting. Features a textured terrain ground with varied height using sine wave combinations, along with the bullseye targets from previous assignments.

### Features Implemented
- **View Modes**: Switch between perspective (orbit) and first-person views.
- **Smooth First-Person Move & Look**: Hold WASD to move, click-drag mouse to look around; motion is frame-rate independent, diagonals normalized, and camera angles use smooth double precision.
- **32-bit BMP Loading**: Make LoadTexBMP() support 32-bit BMPs with alpha channels.
- **Lighting**: Animated light source (rendered as bright sphere) with ambient, diffuse, and specular components. Supports smooth and flat shading modes.
  - **Light Controls**: Adjust light height and distance, pause/resume rotation, manual rotation, and smooth/flat shading toggle.
  - **Normals Debugging**: Toggle display of normals for all objects.
- **Archery Mechanics**:
  - **Shooting**: First-person shooting with charge-up mechanic. Hold right-click to charge power (visualized by dynamic crosshair), release to shoot.
  - **Physics**: Arrows follow physics trajectories with gravity.
- **Objects**:
  - **Ground**: Textured ground terrain with height variations.
  - **Trees**: Procedurally generated trees with textured trunks and alpha-blended leaves.
  - **Bullseyes**: Three textured bullseye targets with animated motion.
  - **Arrow**: Physics-based projectile that can be shot from the camera position.
  - **Light Sphere**: Represents the light source in the scene.

#### Debugging features
- Toggle normals display for objects to visualize lighting effects.

#### Zip File Contents
```bash
zip -r project.zip .
```

---

## Key Bindings

### View Controls
| Key    | Action |
|--------|--------|
| TAB    | Toggle view modes: Perspective (orbit) ↔ First-Person |
| +/-    | Change field of view (perspective modes) |
| 0      | Reset view (camera position/angles, FOV) |
| g/G    | Toggle axes display |
| h/H    | Toggle HUD |
| ESC    | Exit |

### Camera & Interaction Controls
| Key    | Action |
|--------|--------|
| Left-click drag | Look around in first-person (click and drag left mouse to yaw/pitch; works while moving) |
| Right-click | Hold to charge shot (crosshair turns red), release to fire arrow |
| arrows | Look around in perspective (orbit) mode |
| w/s    | Move forward/backward (first-person mode only) |
| a/d    | Strafe left/right (first-person mode only) |

### Lighting Controls
| Key    | Action |
|--------|--------|
| l/L    | Toggle lighting on/off |
| 1/2    | Raise/lower light height |
| 3/4    | Increase/decrease light distance |
| 5      | Pause/resume light rotation |
| 6      | Manual light rotation (when paused) |
| f/F    | Toggle smooth/flat shading |

### Object Controls
| Key    | Action |
|--------|--------|
| p/P    | Pause/resume bullseye motion |
| n/N    | Toggle normals debug lines |

## How to run

```
make        # builds the project
./project   # launch the full scene
```

## Performance Optimizations

- Trunks vs Leaves Split: Scene renders in two passes — opaque trunks/branches first, then transparent leaves. The leaf pass traverses the same hierarchy but emits leaves only (no duplicate trunk geometry), cutting CPU draw calls and overdraw.
- Leaves-Only Path: A `leavesOnly` render mode reuses identical transforms so leaf placement matches the opaque pass exactly, but skips all `glBegin/glEnd` for frustums and normals.
- Culling for Trunks/Branches: Back-face culling is enabled only for the opaque tree pass (both trunk and branch frustums) and set to clockwise front faces during that pass (`glFrontFace(GL_CW)`), then restored. This halves fragment work on the bark geometry without affecting leaves.
- Alpha Test for Leaves: Alpha testing is enabled during the leaf pass (`glAlphaFunc(GL_GREATER, 0.1f)`), discarding fully transparent texels to reduce blending overdraw.
- Reduced State Churn: Leaf texture is bound once for the entire transparent pass; per-leaf `glEnable(GL_TEXTURE_2D)`/`glBindTexture` calls were removed. Per-frustum texture parameter changes were removed from hot loops.
- Disabled GL_NORMALIZE: Normals are pre-normalized for trunks/ground, and lighting is off for the light sphere’s scale. Disabling `GL_NORMALIZE` removes per-vertex renormalization overhead.
- Shared Forest Iterator: Both passes use a shared internal helper to iterate tree rings, avoiding duplicate RNG/setup and keeping placement perfectly consistent across passes.
- Ground Display List + Strips: The terrain mesh is precomputed once (heights + normals) and cached in an OpenGL display list rendered as row-wise `GL_TRIANGLE_STRIP`s. This eliminates per-frame height/normal recomputation and slashes immediate-mode submissions. The list rebuilds only if `steepness`, `size`, `groundY`, or the ground texture changes.
- Swap-Only Present: Removed an explicit `glFlush()` before buffer swap; rely on `glutSwapBuffers()` which flushes implicitly, reducing driver overhead slightly.
- Anisotropic Filtering (if available): Texture loader enables the maximum supported anisotropy via `GL_EXT_texture_filter_anisotropic` for sharper textures at grazing angles.

## Texture credits

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06/22325917510">free seamless texture autumn leaves 2</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06">zaphad1</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06/8595032920">Dirt and Rock Redux</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06">Filter Forge</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/44071822@N08/4727355663">High Quality Tileable Light Wood Texture 1</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/44071822@N08">webtreats</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06/8494824780">Soft Fur</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06">Filter Forge</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06/22030135762">bark 4</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06">zaphad1</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>
