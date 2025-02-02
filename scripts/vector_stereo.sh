#!/bin/bash
exp=$1

if [ -d results/vector_stereo/$exp ]; then
    rm -rf results/vector_stereo/$exp
fi

for i in 0 1 2 # 3 4
do
bin/vector_stereo \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Stereo/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Stereo/VECtor/VECtor.yaml \
    ~/data/VECtor/board-slow \
    ~/data/VECtor/board-slow/rgb/timestamp.txt \
    results/vector_stereo/$exp/vector_stereo_$i/board-slow \
    no_viewer
done

for i in 0 1 2 # 3 4
do
bin/vector_stereo \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Stereo/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Stereo/VECtor/VECtor.yaml \
    ~/data/VECtor/corner-slow \
    ~/data/VECtor/corner-slow/rgb/timestamp.txt \
    results/vector_stereo/$exp/vector_stereo_$i/corner-slow \
    no_viewer
done

for i in 0 1 2 # 3 4
do
bin/vector_stereo \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Stereo/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Stereo/VECtor/VECtor.yaml \
    ~/data/VECtor/corridors-dolly \
    ~/data/VECtor/corridors-dolly/rgb/timestamp.txt \
    results/vector_stereo/$exp/vector_stereo_$i/corridors-dolly \
    no_viewer
done

for i in 0 1 2 # 3 4
do
bin/vector_stereo \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Stereo/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Stereo/VECtor/VECtor.yaml \
    ~/data/VECtor/desk-normal \
    ~/data/VECtor/desk-normal/rgb/timestamp.txt \
    results/vector_stereo/$exp/vector_stereo_$i/desk-normal \
    no_viewer
done

for i in 0 1 2 # 3 4
do
bin/vector_stereo \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Stereo/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Stereo/VECtor/VECtor.yaml \
    ~/data/VECtor/mountain-normal \
    ~/data/VECtor/mountain-normal/rgb/timestamp.txt \
    results/vector_stereo/$exp/vector_stereo_$i/mountain-normal \
    no_viewer
done

for i in 0 1 2 # 3 4
do
bin/vector_stereo \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Stereo/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Stereo/VECtor/VECtor.yaml \
    ~/data/VECtor/robot-normal \
    ~/data/VECtor/robot-normal/rgb/timestamp.txt \
    results/vector_stereo/$exp/vector_stereo_$i/robot-normal \
    no_viewer
done

for i in 0 1 2 # 3 4
do
bin/vector_stereo \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Stereo/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Stereo/VECtor/VECtor.yaml \
    ~/data/VECtor/sofa-normal \
    ~/data/VECtor/sofa-normal/rgb/timestamp.txt \
    results/vector_stereo/$exp/vector_stereo_$i/sofa-normal \
    no_viewer
done
