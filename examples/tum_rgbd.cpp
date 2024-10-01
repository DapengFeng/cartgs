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

#include <torch/torch.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <opencv2/core/core.hpp>
#include <sstream>
#include <thread>

#include "ORB-SLAM3/include/System.h"
#include "include/gaussian_mapper.h"
#include "viewer/imgui_viewer.h"

void LoadImages(const std::string &strAssociationFilename,
                std::vector<std::string> &vstrImageFilenamesRGB,
                std::vector<std::string> &vstrImageFilenamesD,
                std::vector<double> &vTimestamps);
void saveTrackingTime(std::vector<float> &vTimesTrack,
                      const std::string &strSavePath);
void saveGpuPeakMemoryUsage(std::filesystem::path pathSave);

int main(int argc, char **argv) {
  if (argc != 7 && argc != 8) {
    std::cerr << std::endl
              << "Usage: " << argv[0] << " path_to_vocabulary" /*1*/
              << " path_to_ORB_SLAM3_settings"                 /*2*/
              << " path_to_gaussian_mapping_settings"          /*3*/
              << " path_to_sequence"                           /*4*/
              << " path_to_association"                        /*5*/
              << " path_to_trajectory_output_directory/"       /*6*/
              << " (optional)no_viewer"                        /*7*/
              << std::endl;
    return 1;
  }
  bool use_viewer = true;
  if (argc == 8)
    use_viewer = (std::string(argv[7]) == "no_viewer" ? false : true);

  std::string output_directory = std::string(argv[6]);
  if (output_directory.back() != '/') output_directory += "/";
  std::filesystem::path output_dir(output_directory);

  // Retrieve paths to images
  std::vector<std::string> vstrImageFilenamesRGB;
  std::vector<std::string> vstrImageFilenamesD;
  std::vector<double> vTimestamps;
  std::string strAssociationFilename = std::string(argv[5]);
  LoadImages(strAssociationFilename, vstrImageFilenamesRGB, vstrImageFilenamesD,
             vTimestamps);

  // Check consistency in the number of images and depthmaps
  int nImages = vstrImageFilenamesRGB.size();
  if (vstrImageFilenamesRGB.empty()) {
    std::cerr << std::endl << "No images found in provided path." << std::endl;
    return 1;
  } else if (vstrImageFilenamesD.size() != vstrImageFilenamesRGB.size()) {
    std::cerr << std::endl
              << "Different number of images for rgb and depth." << std::endl;
    return 1;
  }

  // Device
  torch::DeviceType device_type;
  if (torch::cuda::is_available()) {
    std::cout << "CUDA available! Training on GPU." << std::endl;
    device_type = torch::kCUDA;
  } else {
    std::cout << "Training on CPU." << std::endl;
    device_type = torch::kCPU;
  }

  // Create SLAM system. It initializes all system threads and gets ready to
  // process frames.
  std::shared_ptr<ORB_SLAM3::System> pSLAM =
      std::make_shared<ORB_SLAM3::System>(argv[1], argv[2],
                                          ORB_SLAM3::System::RGBD);
  float imageScale = pSLAM->GetImageScale();
  std::cout << imageScale << std::endl;

  // Create GaussianMapper
  std::filesystem::path gaussian_cfg_path(argv[3]);
  std::shared_ptr<GaussianMapper> pGausMapper =
      std::make_shared<GaussianMapper>(pSLAM, gaussian_cfg_path, output_dir, 0,
                                       device_type);
  std::thread training_thd(&GaussianMapper::run, pGausMapper.get());

  // Create Gaussian Viewer
  std::thread viewer_thd;
  std::shared_ptr<ImGuiViewer> pViewer;
  if (use_viewer) {
    pViewer = std::make_shared<ImGuiViewer>(pSLAM, pGausMapper);
    viewer_thd = std::thread(&ImGuiViewer::run, pViewer.get());
  }

  // Vector for tracking time statistics
  std::vector<float> vTimesTrack;
  vTimesTrack.resize(nImages);

  std::cout << std::endl << "-------" << std::endl;
  std::cout << "Start processing sequence ..." << std::endl;
  std::cout << "Images in the sequence: " << nImages << std::endl << std::endl;

  // Main loop
  cv::Mat imRGB, imD;
  std::chrono::steady_clock::time_point start_point =
      std::chrono::steady_clock::now();
  for (int ni = 0; ni < nImages; ni++) {
    if (pSLAM->isShutDown()) break;
    // Read image and depthmap from file
    imRGB = cv::imread(std::string(argv[4]) + "/" + vstrImageFilenamesRGB[ni],
                       cv::IMREAD_UNCHANGED);
    cv::cvtColor(imRGB, imRGB, CV_BGR2RGB);
    imD = cv::imread(std::string(argv[4]) + "/" + vstrImageFilenamesD[ni],
                     cv::IMREAD_UNCHANGED);
    double tframe = vTimestamps[ni];

    if (imRGB.empty()) {
      std::cerr << std::endl
                << "Failed to load image at: " << std::string(argv[4]) << "/"
                << vstrImageFilenamesRGB[ni] << std::endl;
      return 1;
    }
    if (imD.empty()) {
      std::cerr << std::endl
                << "Failed to load depth image at: " << std::string(argv[4])
                << "/" << vstrImageFilenamesD[ni] << std::endl;
      return 1;
    }

    if (imageScale != 1.f) {
      int width = imRGB.cols * imageScale;
      int height = imRGB.rows * imageScale;
      cv::resize(imRGB, imRGB, cv::Size(width, height));
      cv::resize(imD, imD, cv::Size(width, height));
    }

    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

    // Pass the image to the SLAM system
    pSLAM->TrackRGBD(imRGB, imD, tframe, std::vector<ORB_SLAM3::IMU::Point>(),
                     vstrImageFilenamesRGB[ni]);

    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

    double ttrack =
        std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1)
            .count();

    vTimesTrack[ni] = ttrack;

    // Wait to load the next frame
    double T = 0;
    if (ni < nImages - 1)
      T = vTimestamps[ni + 1] - tframe;
    else if (ni > 0)
      T = tframe - vTimestamps[ni - 1];

    if (ttrack < T) usleep((T - ttrack) * 1e6);
  }

  // Stop all threads
  pSLAM->Shutdown();
  training_thd.join();
  if (use_viewer) viewer_thd.join();
  std::chrono::steady_clock::time_point end_point =
      std::chrono::steady_clock::now();
  double duration = std::chrono::duration_cast<std::chrono::duration<double>>(
                        end_point - start_point)
                        .count();

  std::cout << "FPS: " << nImages / duration << std::endl;

  // GPU peak usage
  saveGpuPeakMemoryUsage(output_dir / "GpuPeakUsageMB.txt");

  // Tracking time statistics
  saveTrackingTime(vTimesTrack, (output_dir / "TrackingTime.txt").string());

  // Save camera trajectory
  pSLAM->SaveTrajectoryTUM((output_dir / "CameraTrajectory_TUM.txt").string());
  pSLAM->SaveKeyFrameTrajectoryTUM(
      (output_dir / "KeyFrameTrajectory_TUM.txt").string());
  pSLAM->SaveTrajectoryEuRoC(
      (output_dir / "CameraTrajectory_EuRoC.txt").string());
  pSLAM->SaveKeyFrameTrajectoryEuRoC(
      (output_dir / "KeyFrameTrajectory_EuRoC.txt").string());
  pSLAM->SaveTrajectoryKITTI(
      (output_dir / "CameraTrajectory_KITTI.txt").string());

  return 0;
}

void LoadImages(const std::string &strAssociationFilename,
                std::vector<std::string> &vstrImageFilenamesRGB,
                std::vector<std::string> &vstrImageFilenamesD,
                std::vector<double> &vTimestamps) {
  std::ifstream fAssociation;
  fAssociation.open(strAssociationFilename.c_str());
  while (!fAssociation.eof()) {
    std::string s;
    std::getline(fAssociation, s);
    if (!s.empty()) {
      std::stringstream ss;
      ss << s;
      double t;
      std::string sRGB, sD;
      ss >> t;
      vTimestamps.push_back(t);
      ss >> sRGB;
      vstrImageFilenamesRGB.push_back(sRGB);
      ss >> t;
      ss >> sD;
      vstrImageFilenamesD.push_back(sD);
    }
  }
}

void saveTrackingTime(std::vector<float> &vTimesTrack,
                      const std::string &strSavePath) {
  std::ofstream out;
  out.open(strSavePath.c_str());
  std::size_t nImages = vTimesTrack.size();
  float totaltime = 0;
  for (int ni = 0; ni < nImages; ni++) {
    out << std::fixed << std::setprecision(4) << vTimesTrack[ni] << std::endl;
    totaltime += vTimesTrack[ni];
  }

  // std::sort(vTimesTrack.begin(), vTimesTrack.end());
  // out << "-------" << std::endl;
  // out << std::fixed << std::setprecision(4)
  //     << "median tracking time: " << vTimesTrack[nImages / 2] << std::endl;
  // out << std::fixed << std::setprecision(4)
  //     << "mean tracking time: " << totaltime / nImages << std::endl;

  out.close();
}

void saveGpuPeakMemoryUsage(std::filesystem::path pathSave) {
  namespace c10Alloc = c10::cuda::CUDACachingAllocator;
  c10Alloc::DeviceStats mem_stats = c10Alloc::getDeviceStats(0);

  c10Alloc::Stat reserved_bytes =
      mem_stats.reserved_bytes[static_cast<int>(c10Alloc::StatType::AGGREGATE)];
  float max_reserved_MB = reserved_bytes.peak / (1024.0 * 1024.0);

  c10Alloc::Stat alloc_bytes =
      mem_stats
          .allocated_bytes[static_cast<int>(c10Alloc::StatType::AGGREGATE)];
  float max_alloc_MB = alloc_bytes.peak / (1024.0 * 1024.0);

  std::ofstream out(pathSave);
  out << "Peak reserved (MB): " << max_reserved_MB << std::endl;
  out << "Peak allocated (MB): " << max_alloc_MB << std::endl;
  out.close();
}
