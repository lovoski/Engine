# Introduction

`aEngine` is an experimental ECS based game engine.

In ECS architecture, each scene consists of some entities (game object) that has potential hierarchy relations with each other.

These entities store the neccessary indices to components of different types where the actual data is stored (meshes, materials, motions etc.).

During the main loop, all components will get updated by some specific systems. For example, the `NativeScript` component will be maintained by `NativeScriptSystem`.

## How To Compile

The project depends on OpenGL 4.3, FBX SDK 2020.3, ONNX runtime 1.9.0. Other dependencies can be directly retreived with `git clone --recursive ...`.

FBX SDK is required for `Engine/Function/AssetsLoader`, so you need to download the installer [here](https://aps.autodesk.com/developer/overview/fbx-sdk) and install it as autodesk described.

However, ONNX runtime is optional, you can disable it y setting the cmake variable `NN_MODULE` to `OFF`. If you do need this feature, reference `Engine/cmake/FindONNXRuntime.cmake` for more details.

You need to have cmake >= 3.20 to properly execute the cmake compile scripts. After all dependencies are setup, execute `mkdir build && cd build && cmake ..` to configure the project. Or use cmake tools in your ide (vscode) to configure, then build a Release or Debug variant as your need.

## Scripting System

To manipulate the scene with custom function is the core to a game engine. Feel free to check the apis related to scene object manipulation (AddComponent, GetComponent, GetSystemInstance etc.) in file `Engine/Scene.hpp`.

Each native script component can **bind** multiple scriptable derive classes. For example, to write a controller that takes user input and change the property of active camera on the scene:

```cpp
#include "Scene.hpp"
#include "Base/Scriptable.hpp"
class CameraController : public aEngine::Scriptable {
public:
  void Update() override {
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
entity->GetComponent<aEngine::NativeScript>().Bind<CameraController>();
```

When you no onger want this script to execute, you can either set it to disabled or remove it with:
```cpp
entity->GetComponent<aEngine::NativeScript>().Unbind<CameraController>();
```

Each `NativeScript` component can bind multiple scriptables, but each type of scriptable is only allowed one instance.

```cpp
entity->GetComponent<aEngine::NativeScript>().Bind<CameraController>();
// This is allowed, both scriptable will get executed
entity->GetComponent<aEngine::NativeScript>().Bind<CharacterController>();
// This will overwrite previous scriptable of the same type
entity->GetComponent<aEngine::NativeScript>().Bind<CameraController>();
```

Check `Engine/Component/NativeScript.hpp` for more details.

## Render System

...

## Animation System

Currently, the animation system supported import `.bvh` and `.fbx` motion files. After the motion file is loaded, a Skeleton and related Motion will be created as described in `Engine/Function/Animation/Motion`.

As the engine updates all hierarchy at the start of each loop, the local transforms of each joints will be converted to global transforms at the start of each frame, so I updated the local transforms in the `Update` function of `Engine/System/Animation/AnimationSystem`.

When I think about how the skinning should work, I really want the skinning logic to be seperated from the render logic so that I can design a universal render framework that can be applied to all sorts of mesh without writing extra components like `DeformRenderer` etc.

So I decided to use geometry shader as a pre-process step to modify the vertex information before any actual rendering, so I can render the skinned mesh as if it were a normal static mesh.

I created a duplicate of the original VBO to ensure this methods works whne there's more than one instance rendering the mesh data. At the end of each animation system update, I will deform the duplicated VBO with a geometry shader. Then render the deformed VBO with existing render system. More details can be found at `Engine/System/Animation/AnimationSystem` and `Engine/System/Animation/Common`. Here's a breif view of current animation:

<iframe width="1163" height="682" src="https://www.youtube.com/embed/0StVJDh3Bks" title="engine demo 20240823 2" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
