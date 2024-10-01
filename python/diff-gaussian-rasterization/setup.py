#
# Copyright (C) 2023, Inria
# GRAPHDECO research group, https://team.inria.fr/graphdeco
# All rights reserved.
#
# This software is free for non-commercial, research and evaluation use
# under the terms of the LICENSE.md file.
#
# For inquiries contact  george.drettakis@inria.fr
#

from setuptools import setup
from torch.utils.cpp_extension import CUDAExtension, BuildExtension
import os

include_dir = os.path.abspath("../../cuda_rasterizer/")


setup(
    name="diff_gaussian_rasterization",
    packages=["diff_gaussian_rasterization"],
    ext_modules=[
        CUDAExtension(
            name="diff_gaussian_rasterization._C",
            sources=[
                "../../cuda_rasterizer/rasterizer_impl.cu",
                "../../cuda_rasterizer/forward.cu",
                "../../cuda_rasterizer/backward.cu",
                "../../cuda_rasterizer/rasterize_points.cu",
                "../../cuda_rasterizer/adam.cu",
                "../../cuda_rasterizer/conv.cu",
                "ext.cpp",
            ],
            extra_compile_args=["-I" + include_dir],
        )
    ],
    cmdclass={"build_ext": BuildExtension},
)
