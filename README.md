# UAV_UGV

Standalone OMNeT++ project for the UAV/UGV communication simulation and its
post-processing scripts.

This repo is intended to track only the project-specific files. External
frameworks such as OMNeT++, INET, FLORA, and Simu5G should stay outside this
repo and be installed separately.

## Repo Contents

- `src/`: custom NED and C++ Gazebo bridge modules
- `omnetpp.ini`: simulation configurations
- `ground.xml`, `energyConsumptionParameters.xml`: simulation assets
- `path_loss_analysis.py`, `network_metrics_analysis.py`, `analyze_advanced.py`:
  analysis scripts
- `images/`: project-local assets referenced by the visualizer

## External Prerequisites

The generated `Makefile` expects this layout:

```text
workspace/
  UAV_UGV/
  inet4.5/
  flora/
  Simu5G-1.4.2/
```

You also need an OMNeT++ installation available in your shell, for example:

```bash
source /path/to/omnetpp-6.2.0/setenv
```

## Build

From the project directory:

```bash
cd /path/to/UAV_UGV
make -j"$(nproc)"
```

The build links against `../inet4.5`, `../flora`, and `../Simu5G-1.4.2`.

The visualizer also expects the INET 3D assets under `../inet4.5/images/3d/`.

## Run OMNeT

WiFi with Qtenv:

```bash
./UAV_UGV -u Qtenv -f omnetpp.ini -c Communication-GazeboBridge-WiFi -n src:../inet4.5/src
```

5G with Qtenv:

```bash
./UAV_UGV -u Qtenv -f omnetpp.ini -c Communication-GazeboBridge-5G -n src:../inet4.5/src
```

LoRa with Qtenv:

```bash
./UAV_UGV -u Qtenv -f omnetpp.ini -c Communication-GazeboBridge-LoRa -n src:../inet4.5/src:../flora/src
```

WiFi with Cmdenv:

```bash
./UAV_UGV -u Cmdenv -f omnetpp.ini -c Communication-GazeboBridge-WiFi -n src:../inet4.5/src
```

5G with Cmdenv:

```bash
./UAV_UGV -u Cmdenv -f omnetpp.ini -c Communication-GazeboBridge-5G -n src:../inet4.5/src
```

LoRa with Cmdenv:

```bash
./UAV_UGV -u Cmdenv -f omnetpp.ini -c Communication-GazeboBridge-LoRa -n src:../inet4.5/src:../flora/src
```

To keep run outputs separate:

```bash
RUN_TAG=my-run
./UAV_UGV -u Qtenv -f omnetpp.ini -c Communication-GazeboBridge-WiFi \
  -n src:../inet4.5/src \
  --result-dir="results/$RUN_TAG"
```

## Plot Results

Replace `<run_dir>` with the directory used in `--result-dir`.

Path loss and estimated RSSI:

```bash
python3 path_loss_analysis.py --results results/<run_dir> --config WiFi
```

Network metrics:

```bash
python3 network_metrics_analysis.py --results results/<run_dir> --config WiFi
```

Useful crop options for both scripts:

- `--plot 10`
- `--plot ugv-motion`
- `--plot uav-motion`

Example:

```bash
python3 network_metrics_analysis.py \
  --results results/<run_dir> \
  --config WiFi \
  --pathloss-csv results/<run_dir>/Communication-GazeboBridge-WiFi-0_path_loss.csv
```

## Notes

- `Qtenv` is GUI mode and `Cmdenv` is terminal-only.
- LoRa needs the extra NED path `:../flora/src`.
- The analysis scripts write output back into the results directory by default.
