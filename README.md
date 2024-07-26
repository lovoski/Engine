# Engine

An experimental ECS game engine.

## Road maps

- Implement a opengl pbr render system.
- Manage the layout system and gui system
- Create easy to use animation system
- Implement a home made physics system

## ECS architecture

ECS (Entity Component System) atchitecture is a very important thinking in developing this engine. Entity is the actual game object in the scene on which we can attach various components. The components stores the neccessary data needed for the tasks defined in different systems.

Take rendering as an example, each renderable mesh object shold have a `MeshRenderer` component storing the actual mesh data. The texture and other parameters are defined in the component `Material`. When we want to render the entity. We would call the `Update` function defined in `RenderSystem`, use the `EntityManager` singleton to retrieve all the components related to this entity.

The advantage of ECS architecture is that, the component can be stored in a more compact (memory efficient) way. Special mechanics can also be designed to update components with specific types easily.

The main implementation is defined in `src/ecs/base`. The system utilized static variable defined in header files to identify different types. **So functions like `HasComponent` and `GetComponent` can only be called in header files in case the static variables has multiple copies.**

## The rendering

## The gui layout

## The animation

## The physics