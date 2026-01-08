# a3-rens-ws

ReNS (ROS2) workspace for A3 robotics project featuring autonomous navigation and control. 

## 📋 Overview

This repository contains a ROS2 workspace configured for autonomous robot development using the Linorobot2 framework.  It includes packages for robot bringup, teleoperation, SLAM (Simultaneous Localization and Mapping), and micro-ROS integration.

## 🗂️ Repository Structure

```
a3-rens-ws/
├── src/
│   ├── linorobot2/          # Linorobot2 core packages
│   ├── micro_ros_setup/     # Micro-ROS configuration and setup
│   ├── uros/                # Micro-ROS custom packages
│   └── ros2.repos           # Repository dependencies
├── sllidar_ros2/            # SLAMTEC Lidar ROS2 driver
├── scripts/
│   └── launch_all.sh        # Automated launch script for all nodes
├── . gitignore
└── README.md
```

## 🚀 Features

- **Robot Bringup**: Automated robot initialization and hardware interface
- **Teleoperation**: PS3 controller support for manual robot control
- **SLAM Navigation**: Real-time mapping and localization
- **Lidar Integration**:  SLAMTEC Lidar support for environment scanning
- **Micro-ROS**:  Microcontroller integration with ROS2

## 🛠️ Prerequisites

- ROS2 (Humble/Foxy or compatible distribution)
- Ubuntu 20.04/22.04
- `colcon` build tools
- `screen` utility (for running multiple nodes)
- PS3 controller (for teleoperation)

## Authors

- Hein Dekkers ([@heintjepeintje](https://github.com/heintjepeintje))
- Benthe Vermeulen ([@ijsbeer05](https://github.com/ijsbeer05))
- Nicky Buurstee ([@RetroTrack](https://github.com/RetroTrack))
- Abdullah Al Kathiry ([@lateNightCoder200](https://github.com/lateNightCoder200))
- Kyara Mennens ([@gravityjournal](https://github.com/gravityjournal))
- Dimitri van der Hel ([@Dimitri1708](https://github.com/dmitri1708))
