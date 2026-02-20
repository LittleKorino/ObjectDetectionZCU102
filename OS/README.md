# Building a Custom PYNQ Image for ZCU102

> **Objective:** Build a custom PYNQ SD card image for the Xilinx ZCU102 board from source.
> **Toolchain:** Vivado/PetaLinux 2024.1, Docker, PYNQ 3.1.2
> **Status:** ✅ Success

---

## Setup

Following the official [PYNQ SD Card Build Guide](https://pynq.readthedocs.io/en/latest/pynq_sd_card.html).

### 1. Create Directory Structure
```bash
mkdir -p /PYNQ/sdbuild/boards/ZCU102
```

### 2. Configuration Files

Download the BSP: `xilinx-zcu102-v2024.1-05230256.bsp`

Create `ZCU102.spec`:
```makefile
ARCH_ZCU102 := aarch64
BSP_ZCU102 := xilinx-zcu102-v2024.1-05230256.bsp
FPGA_MANAGER_ZCU102 := 1
STAGE4_PACKAGES_ZCU102 := pynq ethernet xrt xrtlib x11
```

### 3. Build Command
```bash
make BOARDDIR=/workspace/sdbuild/boards BOARD=ZCU102
```

> ⚠️ **Do NOT use prebuilt rootfs** — it causes `zocl.ko`/XRT issues (see Pitfalls below).

---

## Known Pitfalls

### ❌ Wrong Toolchain Version (2024.2 vs 2024.1)
The PYNQ build system requires **exactly** Vivado/PetaLinux **2024.1**.
Using 2024.2 causes cascading Yocto codename mismatches (`scarthgap` vs `langdale`).

**Fix:** Install the correct 2024.1 toolchain from scratch. Do not try to patch the Makefiles.

---

### ❌ Device Tree Compile Errors
```text
ERROR: device-tree-1.0-r0 do_compile: Error executing a python function...
Error: .../pynq_xlnk_zynqmp.dtsi:1.1-2 syntax error
FATAL ERROR: Unable to parse input tree
```

**Cause:** Toolchain/Yocto version mismatch corrupting `.dtsi` parsing.  
**Fix:** Use the correct 2024.1 toolchain — this error goes away completely.

---

### ❌ Read-Only Root Filesystem on Boot
Symptoms:
- Every boot requires `mount -o remount,rw /`
- Systemd services fail, network fails
- Kernel panic logs pointing to partition size mismatches

**Cause:** Using a prebuilt rootfs that was not compiled for your specific BSP/setup.  
**Fix:** Build from source without prebuilt overrides (see build command above).

---

### ❌ Using Prebuilt Rootfs (Do Not Do This)
```bash
# DO NOT USE — this causes zocl.ko/XRT issues
make BOARDDIR=/workspace/sdbuild/boards BOARD=ZCU102 \
  PYNQ_SDIST=/workspace/sdbuild/prebuilt/pynq_sdist.tar.gz \
  PYNQ_ROOTFS=/workspace/sdbuild/prebuilt/pynq_rootfs.aarch64.tar.gz
```

The prebuilt rootfs does not have `zocl.ko` properly configured for the ZCU102 BSP.
Always build `xrt` and `zocl` from source.

---

## Result

Build time: ~2 hours  
Output: `ZCU102-3.1.2.img` — fully working PYNQ image with `zocl`, XRT, and FPGA Manager support.

