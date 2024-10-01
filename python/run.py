import os
import time
import numpy as np
import torch
from tqdm import trange
import json
import glob
from renderer import render
from argparse import ArgumentParser
from gaussian_model import GaussianModel
from scipy.spatial.transform import Rotation
from utils import MiniCam, focal2fov

from torchmetrics.image.psnr import PeakSignalNoiseRatio
from torchmetrics.image.ssim import StructuralSimilarityIndexMeasure
from torchmetrics.image.lpip import LearnedPerceptualImagePatchSimilarity

import cv2

from evo.core.trajectory import PoseTrajectory3D
from evo.tools import file_interface
from evo.core import sync
import evo.main_ape as main_ape
from evo.core.metrics import PoseRelation
from evo.tools import plot

import matplotlib.pyplot as plt
import copy

calc_psnr = PeakSignalNoiseRatio().cuda()
calc_ssim = StructuralSimilarityIndexMeasure().cuda()
calc_lpips = LearnedPerceptualImagePatchSimilarity().cuda()


def loadReplica(path):
    color_paths = sorted(glob.glob(os.path.join(path, "results/frame*.jpg")))
    tstamp = [
        float(
            color_path.split("/")[-1]
            .replace("frame", "")
            .replace(".jpg", "")
            .replace(".png", "")
        )
        for color_path in color_paths
    ]
    return color_paths, tstamp


def loadTUM(path):
    if os.path.exists(os.path.join(path, "rgb3")):
        color_paths = sorted(glob.glob(os.path.join(path, "rgb3/*.png")))
    else:
        color_paths = sorted(glob.glob(os.path.join(path, "rgb/*.png")))
    tstamp = [
        float(
            color_path.split("/")[-1]
            .replace("frame", "")
            .replace(".jpg", "")
            .replace(".png", "")
        )
        for color_path in color_paths
    ]
    return color_paths, tstamp


def loadKITTI(path):
    color_paths = sorted(glob.glob(os.path.join(path, "image_2/*.png")))
    tstamp = np.loadtxt(
        os.path.join(path, "times.txt"), delimiter=" ", dtype=np.str_
    ).astype(np.float32)
    return color_paths, tstamp


def loadEuRoC(path):
    color_paths = sorted(glob.glob(os.path.join(path, "mav0/cam0/data/*.png")))
    tstamp = [np.float64(x.split("/")[-1][:-4]) / 1e9 for x in color_paths]
    return color_paths, tstamp


def associate_frames(tstamp_image, tstamp_pose, max_dt=0.08):
    """Pair images, depths, and poses."""
    associations = []
    for i, t in enumerate(tstamp_image):
        j = np.argmin(np.abs(tstamp_pose - t))
        if np.abs(tstamp_pose[j] - t) < max_dt:
            associations.append((i, j))
    return associations


if __name__ == "__main__":
    # Set up command line argument parser
    parser = ArgumentParser(description="evaluation script parameters")
    parser.add_argument("result_path", type=str, default=None)
    parser.add_argument("gt_path", type=str, default=None)
    parser.add_argument("--correct_scale", action="store_true")
    parser.add_argument("--show_plot", action="store_true")
    args = parser.parse_args()
    sh_degree = 3
    gaussians = GaussianModel(sh_degree)
    bg_color = [0, 0, 0]
    background = torch.tensor(bg_color, dtype=torch.float32, device="cuda")
    dirs = os.listdir(args.result_path)
    # load model
    width, height, fovx, fovy = 0, 0, 0, 0
    ts = []
    Rs = []
    render_time = 0
    for file_name in dirs:
        if "shutdown" in file_name:
            iter = file_name.split("_")[0]
            ply_path = os.path.join(
                args.result_path,
                file_name,
                "ply/point_cloud/iteration_{}".format(iter),
                "point_cloud.ply",
            )
            gaussians.load_ply(ply_path)
            with open(
                os.path.join(
                    args.result_path, file_name, "ply", "cameras.json"
                ),
                "r",
            ) as fin:
                camera_paras = json.load(fin)

            width, height, fx, fy = (
                camera_paras[0]["width"],
                camera_paras[0]["height"],
                camera_paras[0]["fx"],
                camera_paras[0]["fy"],
            )
            fovx = focal2fov(fx, width)
            fovy = focal2fov(fy, height)

            render_time = np.loadtxt(
                os.path.join(args.result_path, file_name, "render_time.txt"),
                delimiter=" ",
                dtype=np.str_,
            )
            render_time = render_time[:, 1].astype(np.float32)

    # load gt
    if "replica" in args.gt_path.lower():
        gt_color_paths, gt_tstamp = loadReplica(args.gt_path)
    elif "kitti" in args.gt_path.lower():
        gt_color_paths, gt_tstamp = loadKITTI(args.gt_path)
    elif "euroc" in args.gt_path.lower():
        print(args.gt_path)
        gt_color_paths, gt_tstamp = loadEuRoC(args.gt_path)
    else:
        gt_color_paths, gt_tstamp = loadTUM(args.gt_path)

    #### pose evaluation
    # load estimated poses
    pose_path = os.path.join(args.result_path, "CameraTrajectory_TUM.txt")
    traj_est = file_interface.read_tum_trajectory_file(pose_path)
    # load gt pose
    if "kitti" in args.gt_path.lower():

        def loadKITTIPose(gt_path):
            scene = gt_path.split("/")[-1]
            gt_file = gt_path.replace(scene, "poses/{}.txt".format(scene))
            pose_quat = []
            with open(gt_file, "r") as f:
                lines = f.readlines()
                for i in range(len(lines)):
                    line = lines[i].split()
                    # print(line)
                    c2w = np.array(list(map(float, line))).reshape(3, 4)
                    # print(c2w)
                    quat = np.zeros(7)
                    quat[:3] = c2w[:3, 3]
                    quat[3:] = Rotation.from_matrix(c2w[:3, :3]).as_quat()
                    pose_quat.append(quat)
            pose_quat = np.array(pose_quat)

            return pose_quat

        pose_quat = loadKITTIPose(args.gt_path)
        traj_ref = PoseTrajectory3D(
            positions_xyz=pose_quat[:, :3],
            orientations_quat_wxyz=pose_quat[:, 3:],
            timestamps=np.array(gt_tstamp),
        )

    elif "replica" in args.gt_path.lower():
        gt_file = os.path.join(args.gt_path, "pose_TUM.txt")
        traj_ref = file_interface.read_tum_trajectory_file(gt_file)
    elif "euroc" in args.gt_path.lower():
        gt_file = os.path.join(
            args.gt_path, "mav0/state_groundtruth_estimate0/data.csv"
        )
        traj_ref = file_interface.read_euroc_csv_trajectory(gt_file)
        T_i_c0 = np.array(
            [
                [
                    0.0148655429818,
                    -0.999880929698,
                    0.00414029679422,
                    -0.0216401454975,
                ],
                [
                    0.999557249008,
                    0.0149672133247,
                    0.025715529948,
                    -0.064676986768,
                ],
                [
                    -0.0257744366974,
                    0.00375618835797,
                    0.999660727178,
                    0.00981073058949,
                ],
                [0.0, 0.0, 0.0, 1.0],
            ]
        )
        traj_ref.transform(T_i_c0, True)
    else:
        gt_file = os.path.join(args.gt_path, "groundtruth.txt")
        traj_ref = file_interface.read_tum_trajectory_file(gt_file)

    traj_ref, traj_est = sync.associate_trajectories(
        traj_ref, traj_est, max_diff=0.08
    )
    traj_ref.align(traj_est, True)
    poses = traj_est.poses_se3
    result = main_ape.ape(
        traj_ref,
        traj_est,
        est_name="traj",
        pose_relation=PoseRelation.translation_part,
        align=True,
        correct_scale=args.correct_scale,
    )
    result_rotation_part = main_ape.ape(
        traj_ref,
        traj_est,
        est_name="rot",
        pose_relation=PoseRelation.rotation_part,
        align=True,
        correct_scale=args.correct_scale,
    )

    out_path = os.path.join(args.result_path, "metrics_traj.txt")
    with open(out_path, "w") as fp:
        fp.write(result.pretty_str())
        fp.write(result_rotation_part.pretty_str())
    print(result)

    if args.show_plot:
        traj_est_aligned = copy.deepcopy(traj_est)
        traj_est_aligned.align(traj_ref, correct_scale=True)
        fig = plt.figure()
        traj_by_label = {
            "estimate (not aligned)": traj_est,
            "estimate (aligned)": traj_est_aligned,
            "reference": traj_ref,
        }
        plot.trajectories(fig, traj_by_label, plot.PlotMode.xyz)
        plt.show()

    ## render and evaluation
    tstamp = traj_est.timestamps
    associations = associate_frames(tstamp, gt_tstamp)

    os.makedirs(os.path.join(args.result_path, "image"), exist_ok=True)
    if "_0" in args.result_path:
        os.makedirs(os.path.join(args.result_path, "gt"), exist_ok=True)
    psnr_list, ssim_list, lpips_list, time_list = [], [], [], []
    for index in trange(
        len(associations),
        desc="rendering {}".format(args.result_path.split("/")[-1]),
    ):
        (result_indx, gt_indx) = associations[index]
        w2c = np.linalg.inv(poses[result_indx])
        cam = MiniCam(width, height, fovx, fovy, w2c)
        t0 = time.time()
        render_image = render(cam, gaussians, background)["render"]
        t1 = time.time() - t0
        render_image = render_image.permute(1, 2, 0)
        gt_image = cv2.imread(gt_color_paths[gt_indx], cv2.IMREAD_COLOR)
        gt_image = cv2.cvtColor(gt_image, cv2.COLOR_BGR2RGB)
        if len(gt_image.shape) < 3:
            gt_image = np.broadcast_to(
                gt_image[..., None], (gt_image.shape[0], gt_image.shape[1], 3)
            )

        render_image = torch.clamp(render_image, 0.0, 1.0)
        predict_image_np = render_image.detach().cpu().numpy()
        predict_image_img = np.uint8(predict_image_np * 255)
        predict_image_img = cv2.cvtColor(predict_image_img, cv2.COLOR_BGR2RGB)

        gt_image_torch = (
            torch.from_numpy(np.array(gt_image))
            .to("cuda")
            .permute(2, 0, 1)[None]
            / 255.0
        )
        render_image_torch = render_image.permute([2, 0, 1])[None]
        val_psnr = calc_psnr(render_image_torch, gt_image_torch).item()
        val_ssim = calc_ssim(render_image_torch, gt_image_torch).item()
        val_lpips = calc_lpips(render_image_torch, gt_image_torch).item()

        gt_image = cv2.cvtColor(gt_image, cv2.COLOR_BGR2RGB)
        if "_0" in args.result_path:
            cv2.imwrite(
                os.path.join(
                    args.result_path,
                    "gt",
                    gt_color_paths[gt_indx].split("/")[-1],
                ),
                gt_image,
            )
        cv2.imwrite(
            os.path.join(
                args.result_path,
                "image",
                gt_color_paths[gt_indx].split("/")[-1],
            ),
            predict_image_img,
        )

        psnr_list.append(val_psnr)
        ssim_list.append(val_ssim)
        lpips_list.append(val_lpips)
        time_list.append(t1)

    psnr_list = np.array(psnr_list)
    ssim_list = np.array(ssim_list)
    lpips_list = np.array(lpips_list)
    time_list = np.array(time_list)
    np.savetxt(os.path.join(args.result_path, "psnr.txt"), psnr_list)
    np.savetxt(os.path.join(args.result_path, "ssim.txt"), ssim_list)
    np.savetxt(os.path.join(args.result_path, "lpips.txt"), lpips_list)

    with open(os.path.join(args.result_path, "TrackingTime.txt"), "r") as fin:
        tracking_time = fin.readlines()
    tracking_time = np.array(tracking_time[:-3]).astype(np.float32)

    with open(os.path.join(args.result_path, "eval.txt"), "w") as fout:
        fout.write("psnr: {}\n".format(np.mean(psnr_list)))
        fout.write("ssim: {}\n".format(np.mean(ssim_list)))
        fout.write("lpips: {}\n".format(np.mean(lpips_list)))
        fout.write("tracking s: {}\n".format(np.mean(tracking_time)))
        fout.write("tracking FPS: {}\n".format(1 / np.mean(tracking_time)))

        fout.write("rendering ms: {}\n".format(np.mean(render_time)))
        fout.write("rendering FPS: {}\n".format(1000 / np.mean(render_time)))
