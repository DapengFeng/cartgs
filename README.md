# CaRtGS
### [Homepage](https://dapengfeng.github.io/cartgs/) | [Paper](https://arxiv.org/abs/2410.00486)

**CaRtGS: Computational Alignment for Real-Time Gaussian Splatting SLAM** <br>
[Dapeng Feng](https://github.com/DapengFeng)<sup>1</sup>, [Zhiqiang Chen](https://github.com/thisparticle)<sup>2</sup>, Yizhen Yin<sup>1</sup>, [Shipeng Zhong](https://github.com/zhongshp)<sup>3</sup>, Yuhua Qi<sup>1</sup>, and Hongbo Chen<sup>1</sup> <br>
Sun Yat-Sen University<sup>1</sup>, The University of Hong Kong<sup>2</sup>, WeRide Inc.<sup>3</sup> <br>
<br>
![image](https://github.com/DapengFeng/cartgs/blob/gh_pages/docs/images/teaser.png?raw=true "cartgs")


## Prerequisites

### Dependencies

```
sudo apt install libeigen3-dev libboost-all-dev libjsoncpp-dev libopengl-dev mesa-utils libglfw3-dev libglm-dev
```

## Installation of CaRtGS
``` bash
git clone --recursive https://github.com/DapengFeng/cartgs.git
cd cartgs/
./build.sh
```

## CaRtGS Examples on Some Benchmark Datasets

The benchmark datasets mentioned in our paper: [Replica (NICE-SLAM Version)](https://github.com/cvg/nice-slam) and [TUM RGB-D](https://cvg.cit.tum.de/data/datasets/rgbd-dataset/download).

0. (optional) Download the dataset.
```
scripts/download_replica.sh
scripts/download_tum.sh
```

1. For testing, you could use the below commands to run the system after specifying the `PATH_TO_Replica` and `PATH_TO_SAVE_RESULTS`. We would disable the viewer by adding `no_viewer` during the evaluation.
``` bash
bin/replica_rgbd \
    ORB-SLAM3/Vocabulary/ORBvoc.txt \
    cfg/ORB_SLAM3/RGB-D/Replica/office0.yaml \
    cfg/gaussian_mapper/RGB-D/Replica/replica_rgbd.yaml \
    PATH_TO_Replica/office0 \
    PATH_TO_SAVE_RESULTS
    # no_viewer
```

2. We also provide scripts to conduct experiments on all benchmark datasets mentioned in our paper. We ran each sequence five times to lower the effect of the nondeterministic nature of the system. You need to change the dataset root lines in scripts/*.sh then run:
``` bash
scripts/replica_mono.sh
scripts/replica_rgbd.sh
scripts/tum_mono.sh
scripts/tum_rgbd.sh
# etc.
```



## CaRtGS Evaluation
To use this toolkit, you have to ensure your results on each dataset are stored in the correct format. If you use our `./xxx.sh` scripts to conduct experiments, the results are stored in
```
results
├── replica_mono_0
│   ├── office0
│   ├── ....
│   └── room2
├── replica_rgbd_0
│   ├── office0
│   ├── ....
│   └── room2
│
└── [replica/tum]_[mono/rgbd]_num  ....
    ├── scene_1
    ├── ....
    └── scene_n
```


### Install required python package
``` bash
conda create -n cartgs python=3.10.12 pytorch=2.3.1 torchvision pytorch-cuda=12.1 opencv -c pytorch -c nvidia -c conda-forge
conda activate cartgs
pip install -r python/requirement.txt
```

### install submodel for rendering
``` bash
pip install -e python/diff-gaussian-rasterization/
```

### Convert Replica GT camera pose files to suitable pose files to run EVO package
``` bash
python python/shapeReplicaGT.py --replica_dataset_path PATH_TO_REPLICA_DATASET
```

### To get all metrics, you can run
``` bash
python python/eval.py --dataset_center_path PATH_TO_ALL_DATASET --result_main_folder RESULTS_PATH
```
Finally, you are supposed to get two files including `RESULTS_PATH/log.txt` and `RESULTS_PATH/log.csv`.

## CaRtGS Examples with Real Cameras

We provide an example with the Intel RealSense D455 at `examples/realsense_rgbd.cpp`. Please see `scripts/realsense_d455.sh` for running it.

## Acknowledgement
# Acknowledgement
This work incorporates many open-source codes. We extend our gratitude to the authors of the software.
- [Photo-SLAM](https://github.com/HuajianUP/Photo-SLAM)
- [Taming 3DGS](https://github.com/humansensinglab/taming-3dgs)

# Citation
If you find this work useful in your research, consider citing it:
```
@misc{feng2024CaRtGS,
      title={CaRtGS: Computational Alignment for Real-Time Gaussian Splatting SLAM},
      author={Dapeng Feng and Zhiqiang Chen and Yizhen Yin and Shipeng Zhong and Yuhua Qi and Hongbo Chen},
      year={2024},
      eprint={2410.00486},
      archivePrefix={arXiv},
      primaryClass={cs.CV},
      url={https://arxiv.org/abs/2410.00486},
}
```
