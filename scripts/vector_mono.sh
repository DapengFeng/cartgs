#!/bin/bash
exp=$1

if [ -d results/vector_mono/$exp ]; then
    rm -rf results/vector_mono/$exp
fi

for i in 0 1 2 3 4
do
bin/vector_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Monocular/VECtor/VECtor.yaml \
    ~/data/VECtor/corner-slow \
    ~/data/VECtor/corner-slow/rgb/timestamp.txt \
    results/vector_mono/$exp/vector_mono_$i/corner-slow \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/vector_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Monocular/VECtor/VECtor.yaml \
    ~/data/VECtor/robot-normal \
    ~/data/VECtor/robot-normal/rgb/timestamp.txt \
    results/vector_mono/$exp/vector_mono_$i/robot-normal \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/vector_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/VECtor/VECtor.yaml \
    cfg/gaussian_mapper/Monocular/VECtor/VECtor.yaml \
    ~/data/VECtor/corridors-dolly \
    ~/data/VECtor/corridors-dolly/rgb/timestamp.txt \
    results/vector_mono/$exp/vector_mono_$i/corridors-dolly \
    no_viewer
done
