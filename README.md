# Introduction

`aEngine` is an ECS based game engine aimed to provide a sandbox environment for rapid prototyping.

In ECS architecture, each scene consists of a number of entities (game object) in hierarchy.

These entities store the indices to components which holds the actual data (meshes, materials, motions etc.).

In each main loop, all components are maintained by corresponding systems. For example, the `NativeScript` component will be maintained by `NativeScriptSystem`.

## Features

- Easy to use native scripting system and api.
- Motion capture data structures for `.bvh` and `.fbx`.
- Simple and extensible material system.
- OpenGL based render system for static mesh and skinned mesh.

## How To Compile

The project depends on OpenGL 4.6, ONNX runtime 1.9.0. Other dependencies can be directly retreived with `git clone --recursive ...`.

ONNX runtime is optional, you can disable it by setting the cmake variable `NN_MODULE` to `OFF`. If you do need this feature, reference `Engine/cmake/FindONNXRuntime.cmake` for more details.

You need to have `cmake >= 3.20` to properly execute the cmake scripts.

## Scripting System

To manipulate the scene with custom function is the core to a game engine. Feel free to check the APIs related to scene object manipulation (AddComponent, GetComponent, GetSystemInstance etc.) in file `Engine/Scene.hpp`.

Each native script component can **bind** multiple scriptable derive classes. For example, to write a controller that takes user input and change the property of active camera on the scene:

```cpp
#include "API.hpp"
class CameraController : public aEngine::Scriptable {
public:
  void Update(float dt) override {
    ...
  }
};
```

The `Update` function from Scriptable will get called once each frame. There are other functions you can overload (`LateUpdate`, `DrawToScene` etc.). Check `Engine/Base/Scriptable.hpp` for more details.

After implementing our own controller, we need to **bind** this controller class to some entity on the scene so that it will get executed by `NativeScriptSystem`. For example, in your game code:

```cpp
// GWORLD is a singleton handle for current scene
auto entity = GWORLD.AddNewEntity();
entity->AddComponent<aEngine::NativeScript>();
entity->GetComponent<aEngine::NativeScript>()->Bind<CameraController>();
```

When you no longer want this script to execute, you can either set it to disabled or remove it with:
```cpp
entity->GetComponent<aEngine::NativeScript>()->Unbind<CameraController>();
```

Each `NativeScript` component can bind multiple scriptables, but each type of scriptable is only allowed one instance.

```cpp
entity->GetComponent<aEngine::NativeScript>()->Bind<CameraController>();
// This is allowed, both scriptable will get executed
entity->GetComponent<aEngine::NativeScript>()->Bind<CharacterController>();
// This will overwrite previous scriptable of the same type
entity->GetComponent<aEngine::NativeScript>()->Bind<CameraController>();
```

Check `Engine/Component/NativeScript.hpp` for more details.

## Render System

...

## Animation System

...
