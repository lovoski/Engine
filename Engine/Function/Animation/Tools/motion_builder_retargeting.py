"""
Batched motion retargeting with motion builder, supports mixamo characters and custom bvh skeleton character.
"""

# -*- coding: utf-8 -*-
import os
from pyfbsdk import *

mobuMap = {
    "Reference": "reference",
    "Hips": "Hips",
    "LeftUpLeg": "LeftUpLeg",
    "LeftLeg": "LeftLeg",
    "LeftFoot": "LeftFoot",
    "RightUpLeg": "RightUpLeg",
    "RightLeg": "RightLeg",
    "RightFoot": "RightFoot",
    "Spine": "Spine",
    "LeftArm": "LeftArm",
    "LeftForeArm": "LeftForeArm",
    "LeftHand": "LeftHand",
    "RightArm": "RightArm",
    "RightForeArm": "RightForeArm",
    "RightHand": "RightHand",
    "Head": "Head",
    "LeftShoulder": "LeftShoulder",
    "RightShoulder": "RightShoulder",
    "Neck": "Neck",
    "Spine1": "Spine1",
    "Spine2": "Spine2",
    "Spine3": "Spine3",
    "Spine4": "Spine4",
    "Spine5": "Spine5",
    "Spine6": "Spine6",
    "Spine7": "Spine7",
    "Spine8": "Spine8",
    "Spine9": "Spine9",
    "Neck1": "Neck1",
    "Neck2": "Neck2",
    "Neck3": "Neck3",
    "Neck4": "Neck4",
    "Neck5": "Neck5",
    "Neck6": "Neck6",
    "Neck7": "Neck7",
    "Neck8": "Neck8",
    "Neck9": "Neck9",
    "LeftHandThumb1": "LeftHandThumb1",
    "LeftHandThumb2": "LeftHandThumb2",
    "LeftHandThumb3": "LeftHandThumb3",
    "LeftHandIndex1": "LeftHandIndex1",
    "LeftHandIndex2": "LeftHandIndex2",
    "LeftHandIndex3": "LeftHandIndex3",
    "LeftHandMiddle1": "LeftHandMiddle1",
    "LeftHandMiddle2": "LeftHandMiddle2",
    "LeftHandMiddle3": "LeftHandMiddle3",
    "LeftHandRing1": "LeftHandRing1",
    "LeftHandRing2": "LeftHandRing2",
    "LeftHandRing3": "LeftHandRing3",
    "LeftHandPinky1": "LeftHandPinky1",
    "LeftHandPinky2": "LeftHandPinky2",
    "LeftHandPinky3": "LeftHandPinky3",
    "RightHandThumb1": "RightHandThumb1",
    "RightHandThumb2": "RightHandThumb2",
    "RightHandThumb3": "RightHandThumb3",
    "RightHandIndex1": "RightHandIndex1",
    "RightHandIndex2": "RightHandIndex2",
    "RightHandIndex3": "RightHandIndex3",
    "RightHandMiddle1": "RightHandMiddle1",
    "RightHandMiddle2": "RightHandMiddle2",
    "RightHandMiddle3": "RightHandMiddle3",
    "RightHandRing1": "RightHandRing1",
    "RightHandRing2": "RightHandRing2",
    "RightHandRing3": "RightHandRing3",
    "RightHandPinky1": "RightHandPinky1",
    "RightHandPinky2": "RightHandPinky2",
    "RightHandPinky3": "RightHandPinky3",
    "LeftFootThumb1": "LeftFootThumb1",
    "LeftFootThumb2": "LeftFootThumb2",
    "LeftFootThumb3": "LeftFootThumb3",
    "LeftFootIndex1": "LeftFootIndex1",
    "LeftFootIndex2": "LeftFootIndex2",
    "LeftFootIndex3": "LeftFootIndex3",
    "LeftFootMiddle1": "LeftFootMiddle1",
    "LeftFootMiddle2": "LeftFootMiddle2",
    "LeftFootMiddle3": "LeftFootMiddle3",
    "LeftFootRing1": "LeftFootRing1",
    "LeftFootRing2": "LeftFootRing2",
    "LeftFootRing3": "LeftFootRing3",
    "LeftFootPinky1": "LeftFootPinky1",
    "LeftFootPinky2": "LeftFootPinky2",
    "LeftFootPinky3": "LeftFootPinky3",
    "RightFootThumb1": "RightFootThumb1",
    "RightFootThumb2": "RightFootThumb2",
    "RightFootThumb3": "RightFootThumb3",
    "RightFootIndex1": "RightFootIndex1",
    "RightFootIndex2": "RightFootIndex2",
    "RightFootIndex3": "RightFootIndex3",
    "RightFootMiddle1": "RightFootMiddle1",
    "RightFootMiddle2": "RightFootMiddle2",
    "RightFootMiddle3": "RightFootMiddle3",
    "RightFootRing1": "RightFootRing1",
    "RightFootRing2": "RightFootRing2",
    "RightFootRing3": "RightFootRing3",
    "RightFootPinky1": "RightFootPinky1",
    "RightFootPinky2": "RightFootPinky2",
    "RightFootPinky3": "RightFootPinky3",
    "LeftUpLegRoll": "LeftUpLegRoll",
    "LeftLegRoll": "LeftLegRoll",
    "RightUpLegRoll": "RightUpLegRoll",
    "RightLegRoll": "RightLegRoll",
    "LeftArmRoll": "LeftArmRoll",
    "LeftForeArmRoll": "LeftForeArmRoll",
    "RightArmRoll": "RightArmRoll",
    "RightForeArmRoll": "RightForeArmRoll",
}

candidateMixamoNamespace = [
    "mixamorig:",
    "mixamorig1:",
    "mixamorig2:",
    "mixamorig3:",
    "mixamorig4:",
    "mixamorig5:",
    "mixamorig6:",
    "mixamorig7:",
    "mixamorig8:",
    "mixamorig9:",
]


# -------------- start utils ---------------
def listFiles(directory, extension=".bvh"):
    results = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(extension):
                results.append(file)
    return results


def deselectAll():
    modelList = FBModelList()
    FBGetSelectedModels(modelList, None, True)
    for model in modelList:
        model.Selected = False


def addJointToCharacter(characterObject, slot, jointName):
    myJoint = FBFindModelByLabelName(jointName)
    if myJoint:
        proplist = characterObject.PropertyList.Find(slot + "Link")
        proplist.append(myJoint)


def characterizeBiped(namespace, boneMap):
    app = FBApplication()
    character = FBCharacter(namespace)
    app.CurrentCharacter = character

    # assign Biped to Character Mapping.
    for pslot, pjointName in boneMap.items():
        addJointToCharacter(character, pslot, namespace + pjointName)

    characterized = character.SetCharacterizeOn(True)
    if not characterized:
        print(character.GetCharacterizeError())
    else:
        FBApplication().CurrentCharacter = character
    return character


def plotAnim(char, animChar):
    """
    Receives two characters, sets the input of the first character to the second
    and plot. Return ploted character.
    """
    plotoBla = FBPlotOptions()
    plotoBla.ConstantKeyReducerKeepOneKey = True
    plotoBla.PlotAllTakes = False
    plotoBla.PlotOnFrame = True
    plotoBla.PlotPeriod = FBTime(0, 0, 0, 1)
    plotoBla.PlotTranslationOnRootOnly = True
    plotoBla.PreciseTimeDiscontinuities = True
    # plotoBla.RotationFilterToApply = FBRotationFilter.kFBRotationFilterGimbleKiller
    plotoBla.UseConstantKeyReducer = False
    plotoBla.ConstantKeyReducerKeepOneKey = True
    char.InputCharacter = animChar
    char.InputType = FBCharacterInputType.kFBCharacterInputCharacter
    char.ActiveInput = True
    if not char.PlotAnimation(
        FBCharacterPlotWhere.kFBCharacterPlotOnSkeleton, plotoBla
    ):
        FBMessageBox(
            "Something went wrong",
            "Plot animation returned false, cannot continue",
            "OK",
            None,
            None,
        )
        return False

    return char


def setCurrentTake(desiredName):
    for take in FBSystem().Scene.Takes:
        if take.Name == desiredName:
            FBSystem().CurrentTake = take
            return
    print(f"No take named {desiredName} found")


# -------------- end utils ---------------

skeletonsDirectory = (
    "C:\\Users\\liuwe\\Downloads\\MotionBuilderRetargetingTest\\characters"
)
sourceMotionDirectory = (
    "C:\\Users\\liuwe\\Downloads\\MotionBuilderRetargetingTest\\motions"
)
retargetDirectory = (
    "C:\\Users\\liuwe\\Downloads\\MotionBuilderRetargetingTest\\retarget"
)
if not os.path.exists(retargetDirectory):
    os.mkdir(retargetDirectory)

app = FBApplication()
scene = FBSystem().Scene

sourceMotionFiles = listFiles(sourceMotionDirectory)
targetSkeletonFiles = listFiles(skeletonsDirectory, ".fbx")
globalBVHCounter = 0

# retarget all source motions to all target skeletons
for targetFile in targetSkeletonFiles:
    for motionFile in sourceMotionFiles:
        app.FileNew()
        app.FileMerge(os.path.join(skeletonsDirectory, targetFile), False)
        # select a possible mixamo namespace from candidates
        mixamoNamespace = "mixamorig:"
        for mixamoCandidate in candidateMixamoNamespace:
            candidateHipName = mixamoCandidate + "Hips"
            if FBFindModelByLabelName(candidateHipName):
                mixamoNamespace = mixamoCandidate
                break
        print(f"Use mixamo namespace {mixamoNamespace}")
        character = characterizeBiped(mixamoNamespace, mobuMap)

        deselectAll()
        newTake = FBTake(motionFile)
        scene.Takes.append(newTake)
        setCurrentTake(motionFile)
        newTake.ClearAllProperties(False)
        app.FileImport(os.path.join(sourceMotionDirectory, motionFile), False)
        # set playback fps to 60
        FBPlayerControl().SetTransportFps(FBTimeMode.kFBTimeMode60Frames)
        if globalBVHCounter == 0:
            characterNamespace = "BVH:"
        else:
            globalBVHCounter += 1
            characterNamespace = f"BVH {globalBVHCounter}:"
        motionCharacter = characterizeBiped(characterNamespace, mobuMap)

        # key all frames for bvh to prevent unwanted interpolation between frames
        lEndTime = FBSystem().CurrentTake.LocalTimeSpan.GetStop()
        lEndFrame = FBSystem().CurrentTake.LocalTimeSpan.GetStop().GetFrame()
        lStartFrameTime = FBSystem().CurrentTake.LocalTimeSpan.GetStart()
        lStartFrame = FBSystem().CurrentTake.LocalTimeSpan.GetStart().GetFrame()

        lRange = min(int(lEndFrame) + 1, 50)
        lPlayer = FBPlayerControl()

        for i in range(lRange):
            lPlayer.Goto(FBTime(0, 0, 0, i))
            scene.Evaluate()
            lPlayer.Key()
            scene.Evaluate()

        lPlayer.Goto(FBTime(0, 0, 0, 0))

        plotAnim(character, motionCharacter)

        finalFileName = motionFile + "_" + os.path.splitext(targetFile)[0]
        exportPath = os.path.join(retargetDirectory, finalFileName)
        print(f"Retarget motion {motionFile} to {finalFileName} on {targetFile}")
        character.SelectModels(True, True, True, True)
        app.FileExport(exportPath)
        deselectAll()

    # character.FBDelete()
    # for animCharacter in motionCharacters:
    #     animCharacter.FBDelete()
