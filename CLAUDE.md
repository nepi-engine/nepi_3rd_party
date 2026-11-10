# nepi_3rd_party — Developer Reference

## Purpose

`nepi_3rd_party` vendors third-party libraries that the NEPI platform requires but that are not available through standard package managers in the versions or forms needed. All components here are integrated at source level — either as git submodules or as NEPI-forked repositories. This avoids pip/apt dependency conflicts on embedded NVIDIA Jetson targets and allows NEPI-specific modifications where needed.

## Architecture

```
nepi_3rd_party/
├── ros_numpy/          # ROS ↔ numpy conversion utilities (git submodule)
│   ├── src/ros_numpy/  # Python module: pointcloud, image, occupancy grid conversions
│   ├── package.xml
│   └── CMakeLists.txt
│
└── nepi_yolov5/        # YOLOv5 AI framework integration (git submodule)
    ├── yolov5/         # YOLOv5 source (forked from official repo, nested submodule)
    │   ├── detect.py   # Inference entry point
    │   ├── train.py    # Training entry point
    │   ├── models/     # YOLO model architecture configs
    │   ├── utils/      # Utility functions
    │   └── data/       # Dataset configs (COCO, ImageNet, etc.)
    ├── package.xml     # ROS package wrapper
    └── CMakeLists.txt
```

Both `ros_numpy` and `nepi_yolov5` are git submodules of `nepi_3rd_party`, which is itself a submodule of the `nepi_engine_ws` superproject. `nepi_yolov5/yolov5` is a further nested submodule. Cloning requires `git submodule update --init --recursive`.

Remote origins (NEPI forks, not the upstream repositories):
- `ros_numpy`: `git@github.com:nepi-engine/ros_numpy.git` (branch: `main`)
- `nepi_yolov5`: `git@github.com:nepi-engine/nepi_yolov5.git` (branch: `main`)

## Contents

**ros_numpy**
Provides efficient conversion between ROS message types and numpy arrays. Used throughout NEPI wherever image data, point clouds, or occupancy grids are processed with numpy. The core functions convert `sensor_msgs/Image`, `sensor_msgs/PointCloud2`, and `nav_msgs/OccupancyGrid` to and from numpy arrays without copying data where possible.

**nepi_yolov5**
The legacy YOLO AI framework for NEPI. YOLOv5 predates the `nepi_ai_frameworks` submodule, which now provides YOLOv8 and YOLOv11 adapters. `nepi_yolov5` is retained for backward compatibility with existing deployed models. The NEPI fork wraps the official YOLOv5 source in a ROS catkin package with NEPI-specific configuration and integration points. Active AI development uses `nepi_ai_frameworks` instead.

## Build and Dependencies

Both components are built by catkin as part of the `nepi_engine_ws` workspace. No standalone build.

**ros_numpy:** No additional dependencies beyond the standard ROS stack and numpy. Builds cleanly on Python 3 with ROS 1 (Noetic).

**nepi_yolov5:**
- `rospy`, `std_msgs` (declared in package.xml)
- PyTorch (`torch`, `torchvision`) — required for YOLOv5 inference
- OpenCV (`cv2`), numpy — image processing
- Additional YOLOv5 requirements listed in `nepi_yolov5/yolov5/requirements.txt`

Installing YOLOv5 dependencies: `pip install -r nepi_yolov5/yolov5/requirements.txt` from the workspace.

## Known Constraints and Fragile Areas

**Triple-nested submodules.** `nepi_engine_ws` → `nepi_3rd_party` → `nepi_yolov5` → `yolov5`. A non-recursive submodule init/update will leave inner directories empty with no error message. Always use `git submodule update --init --recursive` when cloning or pulling changes to this submodule.

**nepi_yolov5 is a fork of a fork.** The NEPI YOLOv5 fork is based on the official Ultralytics YOLOv5 repository. NEPI-specific modifications exist in the fork. Rebasing onto upstream YOLOv5 changes risks merge conflicts in modified files. Before pulling upstream updates, check what NEPI has changed in the fork.

**YOLOv5 is legacy.** Active AI framework development is in `nepi_ai_frameworks` (v8, v11). `nepi_yolov5` receives no new development. It is kept for compatibility with existing trained models. New model development should use `nepi_ai_frameworks`.

**SSH-only remotes.** Both submodule remotes use `git@github.com:` URLs. Cloning on a machine without GitHub SSH key access will fail. HTTPS URLs are not configured as an alternative.

**No version pinning.** Both submodules track the `main` branch of NEPI forks. A `git pull` inside these submodules will advance to the latest commit on that branch. The superproject pins a specific commit hash, so workspace builds are reproducible — but manual submodule updates can move ahead of the pinned pointer.

## Decision Log

- 2026-03 — CLAUDE.md created — Initial developer reference, Claude Code authoring pass.
