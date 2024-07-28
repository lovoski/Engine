# The render explaination

`ecs/components/Material` is the class settings most of the shader variables, but the `MVP matrix` are set at the `MeshRenderer` component.

The lights should be passed to `BaseMaterial` component for further processing. The `directional light` will be used to create shader variable `dLightDir<i>` and `dLightColor<i>` where `<i>` is the index of directional light in the scene.

All custom loaded image textures will be named to `imageTex<i>` for the shader to refer to, where `<i>` is the index for the texture among all custom image textures.

The image texture loading can be found at `reesource/ResourceManager`.