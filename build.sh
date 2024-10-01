#!/bin/bash

# current directory
workdir=$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )

# libtorch
if [ ! -d "third_party/libtorch" ]; then
      echo "Downloading libtorch ..."
      wget -c https://download.pytorch.org/libtorch/cu121/libtorch-cxx11-abi-shared-with-deps-2.3.1%2Bcu121.zip
      unzip libtorch-cxx11-abi-shared-with-deps-2.3.1+cu121.zip -d third_party
      rm libtorch-cxx11-abi-shared-with-deps-2.3.1+cu121.zip
fi

# opencv4
echo "Building OpenCV ..."
cmake -B third_party/opencv/build -G Ninja \
      -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc \
      -DCMAKE_CUDA_ARCHITECTURES="native" \
      -DCMAKE_BUILD_TYPE=RELEASE -DWITH_CUDA=ON -DWITH_CUDNN=ON \
      -DWITH_CUFFT=ON -DWITH_CUBLAS=ON -DWITH_NVCUVENC=ON\
      -DOPENCV_DNN_CUDA=ON -DWITH_NVCUVID=ON \
      -DBUILD_TIFF=ON -DBUILD_ZLIB=ON -DBUILD_JASPER=ON -DBUILD_CCALIB=ON \
      -DBUILD_JPEG=ON -DWITH_FFMPEG=ON \
      -DOPENCV_EXTRA_MODULES_PATH=third_party/opencv_contrib/modules \
      -DCMAKE_INSTALL_PREFIX=$workdir/third_party/install/opencv \
      third_party/opencv
cmake --build third_party/opencv/build
cmake --install third_party/opencv/build

# DBoW2
cmake -B third_party/ORB-SLAM3/Thirdparty/DBoW2/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DOpenCV_DIR=$workdir/third_party/install/opencv/lib/cmake/opencv4 \
      third_party/ORB-SLAM3/Thirdparty/DBoW2
cmake --build third_party/ORB-SLAM3/Thirdparty/DBoW2/build

# g2o
cmake -B third_party/ORB-SLAM3/Thirdparty/g2o/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      third_party/ORB-SLAM3/Thirdparty/g2o
cmake --build third_party/ORB-SLAM3/Thirdparty/g2o/build

# Sophus
cmake -B third_party/ORB-SLAM3/Thirdparty/Sophus/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      third_party/ORB-SLAM3/Thirdparty/Sophus
cmake --build third_party/ORB-SLAM3/Thirdparty/Sophus/build

# ORB-SLAM3
echo "Uncompress vocabulary ..."
tar -xf third_party/ORB-SLAM3/Vocabulary/ORBvoc.txt.tar.gz \
    -C third_party/ORB-SLAM3/Vocabulary

cmake -B third_party/ORB-SLAM3/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DOpenCV_DIR=$workdir/third_party/install/opencv/lib/cmake/opencv4 \
      third_party/ORB-SLAM3
cmake --build third_party/ORB-SLAM3/build

# CaRtGS
echo "Building CaRtGS ..."
cmake -B build -G Ninja -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc \
      -DCMAKE_CUDA_ARCHITECTURES="native" \
      -DTorch_DIR=$workdir/third_party/libtorch/share/cmake/Torch \
      -DOpenCV_DIR=$workdir/third_party/install/opencv/lib/cmake/opencv4
cmake --build build
