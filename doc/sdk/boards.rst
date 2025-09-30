.. _zas-boards:

Development Boards
##################

This section describes the Alif development kits supported by this SDK.

Ensemble E7 DevKit (DK-E7)
***************************

The Ensemble E7 DevKit is the only Ensemble development kit available and features the full E7 SoC capabilities.

**Key Features:**

* Two Cortex-M55 CPU cores (RTSS-HE and RTSS-HP)
* Two Ethos-U55 neural network processor cores
* Two Cortex-A32 MPU cores
* Full E7 hardware support

**Important Note:**

The E3 and E5 devices are scaled-down versions of the E7 with fewer cores and features, available as end devices (production silicon).

The DK-E7 is the only Ensemble development kit.
The E7 SoC on the devkit can be configured to emulate these lower-tier devices by disabling cores and features in software, allowing developers to prototype for the entire Ensemble family using a single development kit.

**Use Cases:**

* High-performance embedded applications
* AI/ML workload development and testing
* Multi-core application development
* Prototyping for E7, E5, and E3 target devices

Balletto B1 / Ensemble E1C DevKit (DK-B1)
******************************************

The DK-B1 development kit supports both the Balletto B1 (wireless) and Ensemble E1C (non-wireless) devices. Both devices share the same hardware platform.

**Key Features:**

* Cortex-M55 MCU core
* Ethos-U55 microNPU for AI acceleration
* Bluetooth Low Energy 5.3 (B1 only)
* 802.15.4 based Thread protocols (B1 only)

**Use Cases:**

* Wireless IoT applications (B1)
* Bluetooth LE audio devices (B1)
* Thread/Matter connectivity (B1)
* Low-power AI at the edge (B1 and E1C)
* Compact embedded applications (E1C)

Board Selection
***************

When selecting a development kit:

* Use **DK-E7** for maximum flexibility and performance, or when targeting multiple Ensemble variants
* Use **DK-B1** for wireless connectivity and AI-enabled IoT applications

For detailed hardware specifications, refer to the Alif Hardware Reference Manuals.
