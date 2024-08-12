# Introduction

`aEngine` is an experimental ECS based game engine.

In ECS architecture, each scene consists of some entities (game object) that has potential hierarchy relations with each other.

These entities store the neccessary indices to components of different types where the actual data is stored (meshes, materials, motions etc.).

During the main loop, all components will get updated by some specific systems. For example, the `NativeScript` component will be maintained by `NativeScriptSystem`.

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

..

## Animation System

...
