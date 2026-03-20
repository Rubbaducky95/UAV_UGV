## OMNeT Bridge Handoff

This note summarizes the bridge behavior between ROS2/Gazebo and OMNeT for the `UAV_UGV` project.

### Main Files

- `/home/ruben/omnet_workspace/UAV_UGV/omnetpp.ini`
- `/home/ruben/omnet_workspace/UAV_UGV/src/gazebo/GazeboPositionScheduler.cc`
- `/home/ruben/omnet_workspace/UAV_UGV/src/gazebo/GazeboDrivenMobility.cc`
- `/home/ruben/omnet_workspace/UAV_UGV/src/gazebo/OmnetMetricsServer.cc`
- `/home/ruben/halmstad_ws/src/lrs_halmstad/lrs_halmstad/sim/gazebo_pose_tcp_bridge.py`
- `/home/ruben/halmstad_ws/src/lrs_halmstad/lrs_halmstad/sim/omnet_metrics_bridge.py`

### End-To-End Flow

ROS2/Gazebo -> OMNeT:

1. `gazebo_pose_tcp_bridge` subscribes to ROS2 odometry topics.
2. It stores the latest `(x, y, z, yaw)` for each configured model name.
3. It serves pose snapshots on TCP port `5555`.
4. `GazeboPositionScheduler` connects to that TCP server.
5. Every update interval, it sends `GET` and parses the returned line.
6. It emits `gazeboPoseUpdated` for each model.
7. `GazeboDrivenMobility` listens for those updates and moves the OMNeT nodes.

OMNeT -> ROS2:

1. `OmnetMetricsServer` reads the current UGV/UAV mobility positions.
2. It computes network metrics.
3. It serves one ASCII line per update on TCP port `5556`.
4. `omnet_metrics_bridge` connects to that server.
5. It republishes the metrics as ROS2 topics under `/omnet/*`.

### Pose Snapshot Format

The pose bridge returns either:

- `<count> <name> <x> <y> <z> ...`

or

- `<count> <name> <x> <y> <z> <yaw> ...`

In the current setup, yaw is included.

### Coordinate Mapping

For a tracked model, Gazebo positions are mapped into OMNeT as:

- `x_k = s_x x_k^G + o_x`
- `y_k = s_y y_k^G + o_y`
- `z_k = s_z z_k^G + o_z`

Current bridge configuration in `omnetpp.ini`:

- `s_x = 1`
- `s_y = -1`
- `s_z = 1`
- `o_x = 50`
- `o_y = 50`
- `o_z = 0`

So currently:

- `x_k = x_k^G + 50`
- `y_k = -y_k^G + 50`
- `z_k = z_k^G`

### Velocity

OMNeT velocity is updated from consecutive mapped positions:

- `v_k = (p_k - p_{k-1}) / (t_k - t_{k-1})`

when the position changes and time advances, else:

- `v_k = 0`

### Yaw Mapping

If Gazebo yaw is available:

- `psi_k^map = atan2(s_y sin psi_k^G, s_x cos psi_k^G)`
- `psi_k = wrap_{[-pi,pi]}(psi_k^map + psi_off)`

If Gazebo yaw is absent, orientation falls back to the direction of motion.

### Metrics Computed By OMNeT

Distance:

- `d_k = max(0.1, ||p_{uav,k} - p_{ugv,k}||_2)`

Free-space path loss:

- `FSPL_k = 20 log10(d_k) + 20 log10(f_c) - 147.55`

RSSI:

- `RSSI_k = P_tx - FSPL_k`

SNIR:

- `SNIR_k = RSSI_k - P_noise`

Packet error rate from a sliding window:

- `PER_k = (# dropped events) / M`

Radio-only distance estimate from RSSI inversion:

- `d_k^radio = max(0.1, 10^((P_tx - RSSI_k - 20 log10(f_c) + 147.55)/20))`

### ROS2 Topics Published Back From OMNeT

Published by `omnet_metrics_bridge`:

- `/omnet/sim_time`
- `/omnet/link_distance`
- `/omnet/rssi_dbm`
- `/omnet/snir_db`
- `/omnet/packet_error_rate`
- `/omnet/radio_distance`

### Notation Choice Used In The Thesis Discussion

- Keep superscript `G` only for Gazebo variables.
- Use plain variables for OMNeT-mapped quantities.
- Introduce symbols before equations.
- Avoid extra notation unless it clarifies a real transformation.
