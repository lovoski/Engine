# Engine

An experimental ECS game engine.

## Road maps

### High level goals

- Implement a opengl pbr render system.
- Manage the layout system and gui system
- Create easy to use animation system
- Implement a home made physics system

### Detailed goals

- [x] Create the camera system to manage the movement of camera
- [x] Polish the component gui section, create editor for transform and camera component
- [x] Create a basic material editor, write some actual shader code
- [x] Create hierarchy system for the scene to update all the local transforms
- [ ] Design the filesystem carefully
- [ ] Update the material class, introduce reflection to offer more custom material designs
- [ ] Support multi-pass rendering, allow user control from the material pannel
- [ ] More realistic render effects (PBR, IBL, BRDF ... )
- [ ] Integrate imgui gizmos library, make it easier to manipulate the scene
- [ ] Create the animation system, write code to do the skinning and binding
