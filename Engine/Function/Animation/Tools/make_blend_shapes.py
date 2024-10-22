"""
Create blende shape from poses each frame with blender.
"""

import bpy, re

# get the selected object
selected_object = bpy.context.object

start_frame, end_frame = 1, 52

for i in range(start_frame, end_frame + 1):
    bpy.data.scenes["Scene"].frame_set(i)
    # the modifier indexed 0 should be the armature modifier
    bpy.ops.object.modifier_apply_as_shapekey(
        keep_modifier=True, modifier=selected_object.modifiers[0].name
    )

# get its shapekeys
shape_keys = selected_object.data.shape_keys.key_blocks

# shapekeys reference can eb found at https://hinzka.hatenablog.com/entry/2021/12/21/222635
names = [
    "browInnerUp",
    "browDownLeft",
    "browDownRight",
    "browOuterUpLeft",
    "browOuterUpRight",
    "eyeLookUpLeft",
    "eyeLookUpRight",
    "eyeLookDownLeft",
    "eyeLookDownRight",
    "eyeLookInLeft",
    "eyeLookInRight",
    "eyeLookOutLeft",
    "eyeLookOutRight",
    "eyeBlinkLeft",
    "eyeBlinkRight",
    "eyeSquintLeft",
    "eyeSquintRight",
    "eyeWideLeft",
    "eyeWideRight",
    "cheekPuff",
    "cheekSquintLeft",
    "cheekSquintRight",
    "noseSneerLeft",
    "noseSneerRight",
    "jawOpen",
    "jawForward",
    "jawLeft",
    "jawRight",
    "mouthFunnel",
    "mouthPucker",
    "mouthLeft",
    "mouthRight",
    "mouthRollUpper",
    "mouthRollLower",
    "mouthShrugUpper",
    "mouthShrugLower",
    "mouthClose",
    "mouthSmileLeft",
    "mouthSmileRight",
    "mouthFrownLeft",
    "mouthFrownRight",
    "mouthDimpleLeft",
    "mouthDimpleRight",
    "mouthUpperUpLeft",
    "mouthUpperUpRight",
    "mouthLowerDownLeft",
    "mouthLowerDownRight",
    "mouthPressLeft",
    "mouthPressRight",
    "mouthStretchLeft",
    "mouthStretchRight",
    "tongueOut",
]

# loop through shapekeys and replace the names
for index, key in enumerate(shape_keys):
    if key.name != "Basis":
        try:
            key.name = names[index - 1]
        except:
            pass
