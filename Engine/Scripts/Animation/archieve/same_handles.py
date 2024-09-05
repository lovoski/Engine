# the model takes SkelPosGraphBatch as its input
import argparse
from utils import tensor_utils
import numpy as np
import torch
from mypath import *
from same.mymodel import make_load_model
from same.mydataset import PairedDataset, get_mi_src_tgt_all_graph
from same.skel_pose_graph import SkelPoseGraph, rnd_mask
from utils.skel_gen_utils import create_random_skel
from conversions.graph_to_motion import (
    graph_2_skel,
    hatD_recon_motion,
    gt_recon_motion,
    parse_hatD,
)
from conversions.motion_to_graph import (
    skel_2_graph,
    bvh_2_graph,
    motion_2_states,
    motion_normalize_h2s,
)
from fairmotion.data import bvh
from torch_geometric.data import Batch
from same.mydataset import npz_2_data
from fairmotion.core import motion as motion_class
import ipdb


def load_skeleton_as_graph(bvh_filename):
    """
    load skeleton data from a bvh motion capture file
    returns the skeleton in motion graph
    """
    skeleton_motion = bvh.load(bvh_filename)
    skeleton = skeleton_motion.skel
    # shift root up
    try:
        joint = skeleton.get_joint("LeftToeBase_End")
    except:
        try:
            joint = skeleton.get_joint("LeftToe_End")
        except:
            joint = skeleton.get_joint("LeftFoot_End")
    # joint = skeleton.get_joint("LeftToe")
    root_height = 0
    while joint.name != "Hips":
        root_height -= joint.xform_from_parent_joint[1, 3]
        joint = joint.parent_joint
    # to edit the skeleton in blender
    # make sure you import as `z up` and `y forward`
    root = skeleton.get_joint("Hips")
    root.xform_from_parent_joint[1, 3] = root_height
    root.set_xform_global_recursive(root.xform_from_parent_joint)

    return skel_2_graph(skeleton)


def load_source_motion_sequence_as_graph(bvh_filename, device):
    motion = bvh.load(bvh_filename, ignore_root_skel=True, ee_as_joint=True)
    # normalize the motion
    motion, tpose = motion_normalize_h2s(motion, False)  # normalize motion
    # skel_state, poses_state = motion_2_states
    (lo, go, qb, edges), (q, p, r, pv, qv, pprev, c) = motion_2_states(motion)
    # skeleton data and the motion sequence data
    skel_data, pose_list = npz_2_data(lo, go, qb, edges, q, p, qv, pv, pprev, c, r)
    graph_batch = Batch.from_data_list(
        [SkelPoseGraph(skel_data, pose_i) for pose_i in pose_list]
    )
    return graph_batch.to(device=device), len(pose_list)


def prepare_model_test(model_epoch, device):
    # device, printoptions
    tensor_utils.set_device(device)
    np.set_printoptions(precision=5, suppress=True)
    torch.set_printoptions(precision=5, sci_mode=False)
    torch.backends.cudnn.deterministic = True
    torch.backends.cudnn.benchmark = False

    # Model
    model, cfg = make_load_model(model_epoch, device)
    model.eval()

    load_dir = os.path.join(RESULT_DIR, model_epoch.split("/")[0])
    ms_dict = torch.load(os.path.join(load_dir, "ms_dict.pt"))

    # set SkelPoseGraph class variables
    SkelPoseGraph.skel_cfg = cfg["representation"]["skel"]
    SkelPoseGraph.pose_cfg = cfg["representation"]["pose"]
    SkelPoseGraph.ms_dict = ms_dict

    return model, cfg, ms_dict


def hatD_recon(hatD, tgt_batch, out_rep_cfg, ms_dict, consq_n):
    tgt_root_ids = tgt_batch.ptr[:-1] if hasattr(tgt_batch, "ptr") else [0]
    return parse_hatD(hatD, tgt_root_ids, out_rep_cfg, ms_dict)


def retarget(model, src_batch, tgt_batch, ms_dict, out_rep_cfg, consq_n):
    # src ground truth
    src_motion_list, src_contact_list = gt_recon_motion(src_batch, consq_n)
    # predicted result
    z, hatD = model(src_batch, tgt_batch)
    out = hatD_recon(hatD, tgt_batch, out_rep_cfg, ms_dict, consq_n)
    out_motion_list, out_contact_list = hatD_recon_motion(
        hatD, tgt_batch, out_rep_cfg, ms_dict, consq_n
    )

    # when tgt ground-truth motion is available
    if hasattr(tgt_batch, "q"):
        tgt_motion_list, tgt_contact_list = gt_recon_motion(tgt_batch, consq_n)
        return src_motion_list[0], tgt_motion_list[0], out_motion_list[0]
    else:
        tgt_skel = graph_2_skel(tgt_batch, 1)[0]
        tgt_motion = motion_class.Motion(skel=tgt_skel)
        tpose = np.eye(4)[None, ...].repeat(tgt_skel.num_joints(), 0)
        tpose[0, 1, 3] = tgt_batch.go[0, 1]  # root height
        tgt_motion.add_one_frame(tpose)

        return src_motion_list[0], tgt_motion, out_motion_list[0]


def prepare_source_target_batch(target_skeleton_path, source_motion_path):
    source_graph_batch, nframes = load_source_motion_sequence_as_graph(
        source_motion_path, model.device
    )
    target_skeleton_graph = load_skeleton_as_graph(target_skeleton_path)
    skeleton_graph_batch = Batch.from_data_list([target_skeleton_graph] * nframes).to(
        device=model.device
    )
    return source_graph_batch, skeleton_graph_batch, nframes


def neural_retargeting(target_skeleton_path, source_motion_path):
    source_graph_batch, skeleton_graph_batch, nframes = prepare_source_target_batch(
        target_skeleton_path, source_motion_path
    )
    src_motion, target_motion, out_motion = retarget(
        model,
        source_graph_batch,
        skeleton_graph_batch,
        ms_dict,
        cfg["representation"]["out"],
        nframes,
    )
    return out_motion


model, cfg, ms_dict = prepare_model_test("ckpt0", "cpu")

if __name__ == "__main__":
    target_skeleton_file = "../data/train/character/bvh/abe.bvh"
    # source_motion_file = "../data/retarget/a.bvh"
    # target_skeleton_file = "../data/retarget/Swimming_FR.bvh"
    source_motion_file = "../data/train/motion/bvh/pfnn/LocomotionFlat01_000_0.bvh"

    out_motion = neural_retargeting(target_skeleton_file, source_motion_file)

    bvh.save(out_motion, "output_motion.bvh")
