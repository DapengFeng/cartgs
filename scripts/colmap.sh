#!/bin/bash

bin/train_colmap \
    cfg/colmap/gaussian_splatting.yaml \
    ./data/tandt_db/db/drjohnson \
    results/colmap/drjohnson \
    no_viewer
