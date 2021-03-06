# Honeycomb v0.1
Multi-threaded component-model Android / iOS 3D game engine.

## Documentation
[Project Homepage](http://kometes.github.com/Honeycomb)  
[Honeycomb Library Reference](http://kometes.github.com/Honeycomb/Doxygen/html/annotated.html)

## Roadmap
### Foundations and Modular Framework Phase
The framework is complete and is being actively used/developed, but the features below are on indefinite hold.
    
### Editor Phase (on indefinite hold)
- Move project and code over to QT Creator w/ GCC
- Port framework to Android, get framework test running on phone    -- *PC and Android will share common engine code*
- Setup editor viewport using OpenGL ES 2.0
- 2D rendering interface
    - Shapes                    -- *lines/rects/circles/arcs*
    - Text                      -- *using Freetype*
- 3D multi-viewport, with visible grid and mouse navigation
- Scene object management       -- *create/select objects*
- Transform component           -- *translate/rotate/scale object*
- Cull volume component         -- *visible shape around object*
- Tree component                -- *add/remove children in tree widget*
- NspTree component             -- *visible oct/quad/bsp tree space*
- Flow graph designer
- C++ scripting using Clang and LLVM libs to dynamically compile and run modules
- Task management               -- *generates script and uses dependency flow graph*
- Multi-threaded file manager
- Honeycomb file format with (de)serialization API  -- *fast binary XML-like format*
- Pack file system              -- *using existing open source solution*
- Asset library and manager
    - Convert meshes and animations from Collada into Honeycomb format
    - Convert textures into Honeycomb format        -- *using ImageMagick lib*
- Generic property editor       -- *floats, vectors, colors, transforms*
    - Curve editor for animatable properties
- Mesh component
- Light component
- Material/shader editor        -- *using flow graph*
- Skeletal animation and skinning rendering
- Bone editing
    - modify animation in curve editor
    - blend editor              -- *blend bone groups from different* animations using flow graph
- Texture page editor
    - Texture animations imported from image sequences
- Particle editor               -- *modular and functional particles using a state flow graph*
- Occluder component            -- *for a robust culling system*
- Import lightmaps generated by Blender
- Dynamic shadow properties and rendering
- Physics components            -- *using Bullet Physics*
