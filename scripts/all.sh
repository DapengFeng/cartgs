#!/bin/bash
exp = $1

scripts/replica_mono.sh $exp
scripts/replica_rgbd.sh $exp

scripts/tum_mono.sh $exp
scripts/tum_rgbd.sh $exp
