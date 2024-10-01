#!/bin/bash
exp=$1

if [ -d results/tum_rgbd/$exp ]; then
    rm -rf results/tum_rgbd/$exp
fi

for i in 0 1 2 3 4
do
bin/tum_rgbd \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/RGB-D/TUM/tum_freiburg1_desk.yaml \
    cfg/gaussian_mapper/RGB-D/TUM/tum_rgbd.yaml \
    data/TUM_RGBD/rgbd_dataset_freiburg1_desk \
    cfg/ORB_SLAM3/RGB-D/TUM/associations/tum_freiburg1_desk.txt \
    results/tum_rgbd/$exp/tum_rgbd_$i/rgbd_dataset_freiburg1_desk \
    # no_viewer
done

for i in 0 1 2 3 4
do
bin/tum_rgbd \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/RGB-D/TUM/tum_freiburg2_xyz.yaml \
    cfg/gaussian_mapper/RGB-D/TUM/tum_rgbd.yaml \
    data/TUM_RGBD/rgbd_dataset_freiburg2_xyz \
    cfg/ORB_SLAM3/RGB-D/TUM/associations/tum_freiburg2_xyz.txt \
    results/tum_rgbd/$exp/tum_rgbd_$i/rgbd_dataset_freiburg2_xyz \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/tum_rgbd \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/RGB-D/TUM/tum_freiburg3_long_office_household.yaml \
    cfg/gaussian_mapper/RGB-D/TUM/tum_rgbd.yaml \
    data/TUM_RGBD/rgbd_dataset_freiburg3_long_office_household \
    cfg/ORB_SLAM3/RGB-D/TUM/associations/tum_freiburg3_long_office_household.txt \
    results/tum_rgbd/$exp/tum_rgbd_$i/rgbd_dataset_freiburg3_long_office_household \
    no_viewer
done
