#!/bin/bash
exp=$1

if [ -d results/replica_mono/$exp ]; then
    rm -rf results/replica_mono/$exp
fi

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/office0.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    ~/data/Replica/office0 \
    results/replica_mono/$exp/replica_mono_$i/office0 \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/office1.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    data/Replica/office1 \
    results/replica_mono/$exp/replica_mono_$i/office1 \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/office2.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    data/Replica/office2 \
    results/replica_mono/$exp/replica_mono_$i/office2 \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/office3.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    data/Replica/office3 \
    results/replica_mono/$exp/replica_mono_$i/office3 \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/office4.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    data/Replica/office4 \
    results/replica_mono/$exp/replica_mono_$i/office4 \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/room0.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    data/Replica/room0 \
    results/replica_mono/$exp/replica_mono_$i/room0 \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/room1.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    data/Replica/room1 \
    results/replica_mono/$exp/replica_mono_$i/room1 \
    no_viewer
done

for i in 0 1 2 3 4
do
bin/replica_mono \
    third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/Monocular/Replica/room2.yaml \
    cfg/gaussian_mapper/Monocular/Replica/replica_mono.yaml \
    data/Replica/room2 \
    results/replica_mono/$exp/replica_mono_$i/room2 \
    no_viewer
done
