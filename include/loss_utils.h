/**
 * This file is part of Photo-SLAM
 *
 * Copyright (C) 2023-2024 Longwei Li and Hui Cheng, Sun Yat-sen University.
 * Copyright (C) 2023-2024 Huajian Huang and Sai-Kit Yeung, Hong Kong University
 * of Science and Technology.
 *
 * Photo-SLAM is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Photo-SLAM is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Photo-SLAM. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <torch/torch.h>

#include <vector>

#include "cuda_rasterizer/rasterize_points.h"

namespace loss_utils {

inline torch::Tensor l1_loss(torch::Tensor &network_output,
                             torch::Tensor &gt,
                             const float beta = 1.0f) {
  return torch::abs(network_output - gt).mean();
}

inline torch::Tensor smooth_l1_loss(torch::Tensor &network_output,
                                    torch::Tensor &gt,
                                    const float beta = 1.0f) {
  auto diff = torch::abs(network_output - gt);
  auto loss =
      torch::where(diff < beta, 0.5 * diff.pow(2) / beta, diff - 0.5 * beta);

  return loss.mean();
}

inline torch::Tensor total_variation_loss(torch::Tensor &img) {
  auto diff_i = torch::abs(img.slice(2, 1, img.size(2)) -
                           img.slice(2, 0, img.size(2) - 1));
  auto diff_j = torch::abs(img.slice(1, 1, img.size(1)) -
                           img.slice(1, 0, img.size(1) - 1));

  return diff_i.mean() + diff_j.mean();
}

inline torch::Tensor psnr(torch::Tensor &img1, torch::Tensor &img2) {
  auto mse = torch::pow(img1 - img2, 2).mean();
  return 10.0f * torch::log10(1.0f / mse);
}

/** def psnr(img1, img2):
 *     mse = (((img1 - img2)) ** 2).view(img1.shape[0], -1).mean(1,
 * keepdim=True) return 20 * torch.log10(1.0 / torch.sqrt(mse))
 */
inline torch::Tensor psnr_gaussian_splatting(torch::Tensor &img1,
                                             torch::Tensor &img2) {
  auto mse = torch::pow(img1 - img2, 2)
                 .view({img1.size(0), -1})
                 .mean(1, /*keepdim=*/true);
  return 20.0f * torch::log10(1.0f / torch::sqrt(mse)).mean();
}

inline torch::Tensor gaussian(int window_size,
                              float sigma,
                              torch::DeviceType device_type = torch::kCUDA) {
  std::vector<float> gauss_values(window_size);
  for (int x = 0; x < window_size; ++x) {
    int temp = x - window_size / 2;
    gauss_values[x] = std::exp(-temp * temp / (2.0f * sigma * sigma));
  }
  torch::Tensor gauss =
      torch::tensor(gauss_values, torch::TensorOptions().device(device_type));
  return gauss / gauss.sum();
}

inline torch::autograd::Variable create_window(
    int window_size,
    int64_t channel,
    torch::DeviceType device_type = torch::kCUDA) {
  auto _1D_window = gaussian(window_size, 1.5f, device_type).unsqueeze(1);
  auto _2D_window =
      _1D_window.mm(_1D_window.t()).to(torch::kFloat).unsqueeze(0).unsqueeze(0);
  auto window = torch::autograd::Variable(
      _2D_window.expand({channel, 1, window_size, window_size}).contiguous());
  return window;
}

inline torch::Tensor _ssim(torch::Tensor &img1,
                           torch::Tensor &img2,
                           torch::autograd::Variable &window,
                           int window_size,
                           int64_t channel,
                           bool size_average = true) {
  int window_size_half = window_size / 2;
  auto mu1 =
      torch::nn::functional::conv2d(img1, window,
                                    torch::nn::functional::Conv2dFuncOptions()
                                        .padding(window_size_half)
                                        .groups(channel));
  auto mu2 =
      torch::nn::functional::conv2d(img2, window,
                                    torch::nn::functional::Conv2dFuncOptions()
                                        .padding(window_size_half)
                                        .groups(channel));

  auto mu1_sq = mu1.pow(2);
  auto mu2_sq = mu2.pow(2);
  auto mu1_mu2 = mu1 * mu2;

  auto sigma1_sq =
      torch::nn::functional::conv2d(img1 * img1, window,
                                    torch::nn::functional::Conv2dFuncOptions()
                                        .padding(window_size_half)
                                        .groups(channel)) -
      mu1_sq;
  auto sigma2_sq =
      torch::nn::functional::conv2d(img2 * img2, window,
                                    torch::nn::functional::Conv2dFuncOptions()
                                        .padding(window_size_half)
                                        .groups(channel)) -
      mu2_sq;
  auto sigma12 =
      torch::nn::functional::conv2d(img1 * img2, window,
                                    torch::nn::functional::Conv2dFuncOptions()
                                        .padding(window_size_half)
                                        .groups(channel)) -
      mu1_mu2;

  auto C1 = 0.01 * 0.01;
  auto C2 = 0.03 * 0.03;

  auto ssim_map = ((2 * mu1_mu2 + C1) * (2 * sigma12 + C2)) /
                  ((mu1_sq + mu2_sq + C1) * (sigma1_sq + sigma2_sq + C2));

  if (size_average)
    return ssim_map.mean();
  else
    return ssim_map.mean(1).mean(1).mean(1);
}

inline torch::Tensor ssim(torch::Tensor &img1,
                          torch::Tensor &img2,
                          torch::DeviceType device_type = torch::kCUDA,
                          int window_size = 11,
                          bool size_average = true) {
  auto channel = img1.size(-3);
  auto window = create_window(window_size, channel, device_type);

  // window = window.to(img1.device());
  window = window.type_as(img1);

  return _ssim(img1, img2, window, window_size, channel, size_average);
}

class FusedSSIMMap : public torch::autograd::Function<FusedSSIMMap> {
 public:
  static torch::autograd::tensor_list forward(
      torch::autograd::AutogradContext *ctx,
      const float C1,
      const float C2,
      const torch::Tensor &img1,
      const torch::Tensor &img2) {
    auto ssim_map = fusedssim(C1, C2, img1, img2);
    ctx->save_for_backward({img1, img2});
    ctx->saved_data["C1"] = C1;
    ctx->saved_data["C2"] = C2;
    return {ssim_map};
  }

  static torch::autograd::tensor_list backward(
      torch::autograd::AutogradContext *ctx,
      torch::autograd::tensor_list grad_outputs) {
    auto saved = ctx->get_saved_variables();
    auto img1 = saved[0];
    auto img2 = saved[1];
    auto C1 = static_cast<float>(ctx->saved_data["C1"].toDouble());
    auto C2 = static_cast<float>(ctx->saved_data["C2"].toDouble());
    auto grad = fusedssim_backward(C1, C2, img1, img2, grad_outputs[0]);
    return {torch::Tensor(), torch::Tensor(), grad, torch::Tensor()};
  }
};

inline torch::Tensor fast_ssim(const torch::Tensor &img1,
                               const torch::Tensor &img2,
                               const float C1 = 0.01 * 0.01,
                               const float C2 = 0.03 * 0.03) {
  auto ssim_map = FusedSSIMMap::apply(C1, C2, img1, img2)[0];
  return ssim_map.mean();
}

}  // namespace loss_utils
