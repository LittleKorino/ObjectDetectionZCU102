#  Project: Custom Accelerated CNN Network
## Layer 0: The Ground Work (Building the OS)

> **Context:** This is the ground layer (Step 0) for our custom accelerated CNN project. Before we can accelerate anything, we need a working OS.
> **Objective:** Create a custom PYNQ SD card image for the Xilinx ZCU102 board to serve as the base.
> **Status:** ✅ Success (after a week of pain).

## Step 0.1: The Setup

Following the procedure at [PYNQ SD Card Guide](https://pynq.readthedocs.io/en/latest/pynq_sd_card.html).

1.  **Create Directory Structure:**
    ```bash
    mkdir -p /PYNQ/sdbuild/boards/ZCU102
    ```
2.  **Configuration:**
    *   Download BSP: `xilinx-zcu102-v2024.1-05230256.bsp`
    *   Create `ZCU102.spec`:

    ```makefile
    ARCH_ZCU102 := aarch64
    BSP_ZCU102 := xilinx-zcu102-v2024.1-05230256.bsp
    FPGA_MANAGER_ZCU102 := 1
    STAGE4_PACKAGES_ZCU102 := pynq ethernet xrt xrtlib x11
    ```

3.  **The (Intended) Build Command:**
    ```bash
    make BOARDDIR=/workspace/sdbuild/boards BOARD=ZCU102
    ```

---

## DevLog: The "Version Mismatch" Nightmare

### Phase 1: The Hubris (Vivado 2024.2 vs 2024.1)
After setting up Docker and running `make`, we hit the wall immediately.
**Surprise, surprise:** It requires the **2024.1** toolchain. We were running **2024.2**.

Did we do the sensible thing and install the correct version? **No.** We decided to fight the Makefile.

    *   **The Hack:** We manually found and replaced all occurrences of `2024.2` string with `2024.1`.
    *   **The Result:** The script continued... until it crashed during the PetaLinux project creation.
    *   It expected `langdale` (Yocto codename for 2024.1).
    *   We gave it `scarthgap` (2024.2).
    *   **The Second Hack:** Replaced `langdale` with `scarthgap` everywhere.

*Mistakes were just piling up. The foreshadowing was palpable.*

### Phase 2: The "Device Tree" Hell
Just when we thought we had outsmarted the system:
```text
ERROR: device-tree-1.0-r0 do_compile: Error executing a python function...
Error: .../pynq_xlnk_zynqmp.dtsi:1.1-2 syntax error
FATAL ERROR: Unable to parse input tree
```
**Status:** Stuck here for **DAYS** (about a week!).  
**Silver Lining:** I learned way more about device trees and overlays than I ever wanted to know.

### Phase 3: The "Fix" (Making it Worse)
I eventually gave up on the `zocl` and `uio` errors.
*   **My "Solution":** I modified the `.dtsi` file to completely ignore `zocl` and `uio`. "I'll just use the built-in `fpga_manager` later," I lied to myself.
*   **The Output:** I got a `ZCU102-3.1.2.img`.
*   **The Reality:** I immediately regretted my decision.

Booting in QEMU showed the root partition was **Read-Only**.
*   Every boot required remounting `/` as `rw`.
    *   Systemd services failed. Network failed.
    *   **Cause:** Kernel panic due to partition header/image size mismatches.
    *   **Result:** Battled this zombie image for another 2 days.

---

## The Redemption: Doing It Right

### Surrendering to 2024.1
I finally admitted defeat. I installed the **Vivado 2024.1** toolchain from scratch and rebuilt the Docker container.

But wait—it still wasn't over.
The rootfs *still* had problems. `zocl.ko` and XRT were having a hard time.

### The Realization
I realized I had been using the cached prebuilts to save time:
```bash
# The command that betrayed me
make BOARDDIR=/workspace/sdbuild/boards BOARD=ZCU102 \
 PYNQ_SDIST=/workspace/sdbuild/prebuilt/pynq_sdist.tar.gz \
 PYNQ_ROOTFS=/workspace/sdbuild/prebuilt/pynq_rootfs.aarch64.tar.gz
```
**The Plot Twist:** The prebuilt rootfs didn't have the `zocl.ko` module properly configured for my specific setup (somehow).

### The Final Solution
I stripped the command down to force it to compile `xrt` and `zocl` from source:

```bash
make BOARDDIR=/workspace/sdbuild/boards BOARD=ZCU102
```

It took **2 hours**.
But out came the golden artifact: **A fully working `ZCU102-3.1.2.img`**.

### Victory!
It works properly. Lesson learned: Just install the right toolchain next time.
