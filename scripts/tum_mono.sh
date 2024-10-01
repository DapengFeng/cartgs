#!/bin/bash
exp=$1

if [ -d results/tum_mono/$exp ]; then
    rm -rf results/tum_mono/$exp
fi

for i in 0 1 2 3 4
do
bin/tum_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/TUM/tum_freiburg1_desk.yaml \
    cfg/gaussian_mapper/Monocular/TUM/tum_mono.yaml \
    data/TUM_RGBD/rgbd_dataset_freiburg1_desk \
    results/tum_mono/$exp/tum_mono_$i/rgbd_dataset_freiburg1_desk \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/tum_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/TUM/tum_freiburg2_xyz.yaml \
    cfg/gaussian_mapper/Monocular/TUM/tum_mono.yaml \
    data/TUM_RGBD/rgbd_dataset_freiburg2_xyz \
    results/tum_mono/$exp/tum_mono_$i/rgbd_dataset_freiburg2_xyz \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/tum_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/TUM/tum_freiburg3_long_office_household.yaml \
    cfg/gaussian_mapper/Monocular/TUM/tum_mono.yaml \
    data/TUM_RGBD/rgbd_dataset_freiburg3_long_office_household \
    results/tum_mono/$exp/tum_mono_$i/rgbd_dataset_freiburg3_long_office_household \
    no_viewer
done
