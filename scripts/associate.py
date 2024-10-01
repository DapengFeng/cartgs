# Software License Agreement (BSD License)
#
# Copyright (c) 2013, Juergen Sturm, TUM
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of TUM nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Requirements:
# sudo apt-get install python-argparse
"""The Kinect provides the color and depth images in an un-synchronized way.

This means that the set of time stamps from the color images do not
intersect with those of the depth images. Therefore, we need some way of
associating color images to depth images.

For this purpose, you can use the ''associate.py'' script. It reads the
time stamps from the rgb.txt file and the depth.txt file, and joins them
by finding the best matches.
"""

import argparse
import os
import numpy as np


def parse_list(filepath, skiprows=0):
    """Read list data."""
    return np.loadtxt(
        filepath, delimiter=" ", dtype=np.unicode_, skiprows=skiprows
    )


def associate_frames(tstamp_image, tstamp_depth, tstamp_pose, max_dt=0.08):
    """Pair images, depths, and poses."""
    associations = []
    for i, t in enumerate(tstamp_image):
        if tstamp_pose is None:
            j = np.argmin(np.abs(tstamp_depth - t))
            if np.abs(tstamp_depth[j] - t) < max_dt:
                associations.append((i, j))
        else:
            j = np.argmin(np.abs(tstamp_depth - t))
            k = np.argmin(np.abs(tstamp_pose - t))
            if (np.abs(tstamp_depth[j] - t) < max_dt) and (
                np.abs(tstamp_pose[k] - t) < max_dt
            ):
                associations.append((i, j))
    return associations


if __name__ == "__main__":
    # parse command line
    parser = argparse.ArgumentParser(
        description="""
    This script takes two data files with timestamps and associates them
    """
    )
    parser.add_argument("datapath", help="datapath")
    parser.add_argument(
        "--frame_rate", help="frame rate (default: 32)", default=32
    )
    parser.add_argument(
        "--max_difference",
        help="maximally allowed time difference for matching entries (default: 0.08)",
        default=0.08,
    )
    args = parser.parse_args()

    datapath = args.datapath
    frame_rate = args.frame_rate

    image_list = os.path.join(datapath, "rgb.txt")
    depth_list = os.path.join(datapath, "depth.txt")
    pose_list = os.path.join(datapath, "groundtruth.txt")

    image_data = parse_list(image_list)
    depth_data = parse_list(depth_list)
    pose_data = parse_list(pose_list, skiprows=1)

    tstamp_image = image_data[:, 0].astype(np.float64)
    tstamp_depth = depth_data[:, 0].astype(np.float64)
    tstamp_pose = pose_data[:, 0].astype(np.float64)
    associations = associate_frames(tstamp_image, tstamp_depth, tstamp_pose)

    indicies = [0]
    for i in range(1, len(associations)):
        t0 = tstamp_image[associations[indicies[-1]][0]]
        t1 = tstamp_image[associations[i][0]]
        if t1 - t0 > 1.0 / frame_rate:
            indicies += [i]

    for ix in indicies:
        (a, b) = associations[ix]
        print(
            "%f %s %f %s"
            % (
                tstamp_image[a],
                image_data[a, 1],
                tstamp_depth[b],
                depth_data[b, 1],
            )
        )
