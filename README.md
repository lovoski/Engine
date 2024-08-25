# Introduction

`aEngine` is an ECS based game engine aimed to provide a sandbox environment for rapid prototyping.

In ECS architecture, each scene consists of some entities (game object) that has potential hierarchy relations with each other.

These entities store the neccessary indices to components of different types where the actual data is stored (meshes, materials, motions etc.).

During the main loop, all components will get updated by some specific systems. For example, the `NativeScript` component will be maintained by `NativeScriptSystem`.

Another thing worth mentioning is that, all component instances are stored as `shaderd_ptr` in relative `ComponentLists<T>`. The reason is that the copy operation between components is relative common, so there might be some issue with the life cycle of a component. If I put a opengl buffer object in a component and deconstruct it at the components's deconstructor, the copy of this component won't be able to access this buffer (like `skeletonMatrices` in `DeformRenderer`).

## Features

- Easy to use native scripting system and api.
- Motion capture data structures for `.bvh` and `.fbx`.
- Simple and extensible material system.
- OpenGL based render system for static mesh and skinned mesh.

## How To Compile

The project depends on OpenGL 4.3, FBX SDK 2020.3, ONNX runtime 1.9.0. Other dependencies can be directly retreived with `git clone --recursive ...`.

FBX SDK is required for `Engine/Function/AssetsLoader`, so you need to download the installer [here](https://aps.autodesk.com/developer/overview/fbx-sdk) and install it as autodesk described.

However, ONNX runtime is optional, you can disable it by setting the cmake variable `NN_MODULE` to `OFF`. If you do need this feature, reference `Engine/cmake/FindONNXRuntime.cmake` for more details.

You need to have `cmake >= 3.20` to properly execute the cmake compile scripts. After all dependencies are setup, execute `mkdir build && cd build && cmake ..` to configure the project. Or use cmake tools in your ide (vscode) to configure, then build a Release or Debug variant as your need.

## Scripting System

To manipulate the scene with custom function is the core to a game engine. Feel free to check the APIs related to scene object manipulation (AddComponent, GetComponent, GetSystemInstance etc.) in file `Engine/Scene.hpp`.

Each native script component can **bind** multiple scriptable derive classes. For example, to write a controller that takes user input and change the property of active camera on the scene:

```cpp
#include "Scene.hpp"
#include "Base/Scriptable.hpp"
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
