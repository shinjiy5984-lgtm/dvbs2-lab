# dvbs2-lab
Raspberry Pi DVB-S2 experiment

# dvbs2-lab
Headless DVB-S2 beacon experiment using GNU Radio 3.10 + gr-dvbs2.  
Validated on Raspberry Pi 5 (64-bit).

---

## Power-Cycle Validation

The beacon successfully restarted **3 consecutive times** under repeated:

- Power OFF
- Power ON

No manual intervention required.

This confirms:

- FIFO recreation works
- No orphan processes remain
- Beacon restarts cleanly
- Headless operation is stable

---

## GUI Flowgraph Warning

### RF_UDP_dvbs2_tx.grc

Opening this flowgraph in GNU Radio and generating Python code will produce a **GUI-based transmitter**.

This version:

- Launches a graphical interface
- Is not suitable for SSH / headless operation

---

## Recommended: CLI Version

### RF_FIFO_dvbs2_tx_rx.py

This script:

- Removes all GUI components
- Is designed for CLI execution
- Is suitable for SSH-based headless systems
- Is strongly recommended for beacon operation

---

## Step 1 — Generate Test TS (Run Once)

Create a DVB-S2 test stream file for beacon operation:

```bash
mkdir -p ~/dvb-s/data
cd ~/dvb-s/data

ffmpeg -y \
  -f lavfi -i testsrc2=size=800x480:rate=15 \
  -f lavfi -i sine=frequency=1000:sample_rate=22050 \
  -vf format=yuv420p \
  -c:v libx264 -preset veryfast -tune zerolatency \
  -profile:v baseline -level 3.0 \
  -g 30 -keyint_min 30 -sc_threshold 0 \
  -b:v 120k -maxrate 150k -bufsize 300k \
  -c:a mp2 -ac 1 -ar 22050 -b:a 24k \
  -t 240 \
  -muxrate 333k \
  -f mpegts out_800x480_av_mp2.ts
```

## Beacon Startup Procedure
### Window 1 – Preparation

```bash
mkdir /tmp/jpg
mkfifo /tmp/in.ts
ffmpeg -y -f lavfi -i color=size=640x480:rate=1:color=black \
  -frames:v 1 -update 1 /tmp/jpg/latest.jpg
```

### Window 2 – Monitor

```bash
./dvb-s/monitor.sh
```

Displays `/tmp/jpg/latest.jpg` at 1 fps using `feh` with watchdog restart.

### Window 3 – Beacon

```bash
cd dvb-s
./autobeacon.sh
```
### Features

- FIFO recreated every cycle
- `dvbs2` process auto-restart
- Dummy writer to prevent FIFO EOF
- 180 sec TX + 60 sec idle loop
- Full cleanup on SIGINT/SIGTERM
- Power-cycle tolerant design

### Beacon Cycle Structure

1. Prepare FIFO
2. Start `dvbs2`
3. Start dummy writer
4. Start `ffmpeg` (TS injection + 1 fps JPEG monitor)
5. Wait TX duration
6. Cleanup processes
7. Sleep
8. Repeat

### Repository Structure

- `scripts/` — beacon and monitoring scripts
- `docs/` — operational notes
- `configs/` — safe public configuration examples

### Status

- ✅ 24-hour endurance test completed

### Notes

This repository focuses on operational stability of DVB-S2 beacon systems under constrained hardware environments.

Designed for reproducible headless operation.


---

## この README について

- 技術的に十分具体的
- 余計な個人情報なし
- 公開しても安全
- エンジニアが読めばすぐ理解できる構成
