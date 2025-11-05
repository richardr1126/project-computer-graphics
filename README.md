# Final Project - CSCI5229 Computer Graphics

**Richard Roberson**
CSCI5229 Fall 2025

Implements texture mapping on a 3D scene with lighting. Features a textured terrain ground with varied height using sine wave combinations, along with the bullseye targets from previous assignments.

### Features Implemented
- **View Modes**: Switch between perspective (orbit), first-person, and orthographic views.
- **Smooth First-Person Move & Look**: Hold WASD to move, click-drag mouse to look around; motion is frame-rate independent, diagonals normalized, and camera angles use smooth double precision.
- **Lighting**: Animated light source (rendered as bright sphere) with ambient, diffuse, and specular components. Supports smooth and flat shading modes.
  - **Light Controls**: Adjust light height and distance, pause/resume rotation, manual rotation, and smooth/flat shading toggle.
  - **Normals Debugging**: Toggle display of normals for all objects.
- **Objects**:
  - **Ground**: Textured ground terrain with height variations.
  - **Bullseyes**: Three textured bullseye targets with animated motion.
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
| TAB    | Cycle view modes: Orthographic → Perspective (orbit) → First-Person |
| +/-    | Change field of view (perspective modes) |
| 0      | Reset view (camera position/angles, FOV) |
| g/G    | Toggle axes display |
| h/H    | Toggle HUD |
| ESC    | Exit |

### Camera Controls
| Key    | Action |
|--------|--------|
| mouse drag | Look around in first-person (click and drag left mouse to yaw/pitch; works while moving) |
| arrows | Look around in orthographic/perspective (orbit) modes |
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

### Builder-only Controls
| Key    | Action |
|--------|--------|
| t/T    | Toggle ground on/off |

## How to run

From the `project/` folder:

```
make        # builds both 'project' and 'builder'
./project   # launch the full scene
```

## Texture credits

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06/22325917510">free seamless texture autumn leaves 2</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/25797459@N06">zaphad1</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/44071822@N08/4727355663">High Quality Tileable Light Wood Texture 1</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/44071822@N08">webtreats</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>

<p class="attribution">"<a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06/8494824780">Soft Fur</a>" by <a rel="noopener noreferrer" href="https://www.flickr.com/photos/93421824@N06">Filter Forge</a> is licensed under <a rel="noopener noreferrer" href="https://creativecommons.org/licenses/by/2.0/?ref=openverse">CC BY 2.0 <img src="https://mirrors.creativecommons.org/presskit/icons/cc.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /><img src="https://mirrors.creativecommons.org/presskit/icons/by.svg" style="height: 1em; margin-right: 0.125em; display: inline;" /></a>.</p>
