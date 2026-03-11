<img width="1920" height="1080" alt="TX and RX" src="https://github.com/user-attachments/assets/03435dcf-e216-4551-ae3f-bd9e9535d6f7" />


# Minimal DVB-S2 Laboratory

A fully reproducible DVB-S2 experiment environment built from a clean Raspberry Pi OS installation.

Target users

This project is intended for researchers, students and SDR experimenters.

It assumes familiarity with:
```text
• Linux environment
• GNU Radio
• SDR concepts
• RF experimentation
```
This is not a beginner-friendly project.　
# Goal

Provide the cheapest reproducible DVB-S2 research environment.
```text
Hardware:
Raspberry Pi 5
RTL-SDR
```
```text
Video Source
      │
      │
   MPEG-TS
      │
      │
GNU Radio DVB-S2 TX
      │
      │
Pluto+
      │
      │
      RF
      │
      │
RTL-SDR
      │
      │
GNU Radio DVB-S2 RX
      │
      │
UDP stream
      │
      │
Video Player
```

# Students can reproduce the system with minimal cost.


# DVB-S2 Auto Beacon

## Dependencies

```text
GNU Radio 3.10
gr-dvbs2
gr-dvbs2rx
ffmpeg
RTL-SDR (SoapySDR)
```

## Environment
This project is tested only on Raspberry Pi OS (64bit).
```text
Test environment:
    • Raspberry Pi 5
    • Raspberry Pi OS 64bit
    • GNU Radio 3.10
    • Headless operation via SSH
⚠️ Ubuntu is NOT tested and not supported.
```

## Environment Setup (Raspberry Pi OS)
```text
✔ Tested environment
✔ Not supported environment
```
# RTL-SDR setup
write down and save the file and reboot 
```bash
sudo vi /etc/modprobe.d/blacklist-rtl.conf
```

```bash
blacklist dvb_usb_rtl28xxu
blacklist rtl2832
blacklist rtl2830
blacklist dvb_usb_v2
```

```bash
sudo apt update
sudo apt install rtl-sdr
```

# 1 Install dependencies

```bash
cd ~

sudo apt update

sudo apt install -y \
  build-essential cmake pkg-config \
  git wget curl \
  python3 python3-pip python3-numpy python3-mako python3-yaml \
  python3-click python3-click-plugins \
  libboost-all-dev \
  libfftw3-dev \
  libgmp-dev \
  libusb-1.0-0-dev \
  libudev-dev \
  liborc-0.4-dev \
  libspdlog-dev
```
# 2 Documentation tools  
```bash
sudo apt install -y doxygen graphviz
```
# 3 Additional dependency
```bash
sudo apt install -y libpcap-dev
```
# 4 Optimize VOLK
```bash
volk_profile
```
# 5 Install GNU Radio
```bash
sudo apt install -y gnuradio gnuradio-dev
```
# 6 Clone repositories
```bash
cd ~

mkdir src
cd src

git clone https://github.com/drmpeg/gr-dvbs2.git
EXtract gr-dvbs2rx.7z
```
# 7 Build gr-dvbs2
```bash
cd ~/src/gr-dvbs2

mkdir build
cd build

cmake ..
make -j$(nproc)

sudo make install
sudo ldconfig
```
# 8 Build gr-dvbs2rx
```bash
cd ~/src/gr-dvbs2rx

mkdir build
cd build

cmake ..
make -j$(nproc)

sudo make install
sudo ldconfig
```

# DVB-S2 Lab
This project provides a complete DVB-S2 SDR receiver chain
for education and experimentation.

This repository contains a reproducible DVB-S2 beacon
transmitter and receiver experiment built with GNU Radio.

The system runs on Raspberry Pi and uses RTL-SDR for reception.

Features:

## System Flow
```text
• Automatic DVB-S2 beacon transmitter
• GNU Radio DVB-S2 receiver
• UDP TS streaming
• Power-cycle restart validation
```
## System Flow

Signal path of the DVB-S2 experiment:
```text
DVB-S2 TX
    ↓ RF
RTL-SDR
    ↓
GNU Radio DVB-S2 RX
    ↓
tmp.ts
    ↓
ffmpeg
    ↓
UDP
    ↓
ffplay
```


Version: v0.9 (Experimental)

## Tested Environment
- Raspberry Pi 5 (64-bit OS)
- Pluto+
- GNU Radio 3.10

---

## Default Settings

Default transmit frequency:
1295000000 Hz (1295 MHz)

---

## Manual Usage

@1295 MHz
```bash
./autobeacon.sh
```

@1298 MHz
```bash
./autobeacon.sh 1298000000
```

## Directory Structure

```
~/dvb-s/
├── RF_FIFO_dvbs2_tx_rx.py
├── RF_UDP_dvbs2_tx.grc
├── autobeacon.sh
├── monitor.sh
├── RF_FIFO_dvbs2_tx_rx.py
├── RF_UDP_dvbs2_rx.py
├── RF_UDP_dvbs2_tx.grc
├── dvbs2rx_rx_hier.grc
├── RX.sh
├── gr-dvbs2rx.7z
├── data/
│   ├── out_800x480_av_mp2.ts
│   └── udp.out
└── README.md
```

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
### Installing the dvbs2rx_rx_hier GNU Radio Block

The file dvbs2rx_rx_hier.grc defines a hierarchical GNU Radio block used by the DVB-S2 receiver.
To install and use this block:

### 1. Clone the repository
```bash
git clone https://github.com/shinjiy5984-lgtm/dvbs2-lab.git
cd dvbs2-lab
```

### 2. Copy the block definition

GNU Radio loads custom hierarchical blocks from the user state directory.
Copy the .grc file to the GNU Radio state directory:
```bash
mkdir -p ~/.local/state/gnuradio
cp dvbs2rx_rx_hier.grc ~/.local/state/gnuradio/
```
### 3. Start GNU Radio Companion
```bash
gnuradio-companion
```

You can also generate the Python implementation directly:
```bash
gnuradio-companion dvbs2rx_rx_hier.grc
```
Press F5 to generate the Python code.
### Press F6.
The block dvbs2rx_rx_hier will now be available for use.

### DVB-S2 software receiver Operation Sequence

### The order of operations is important.

### Window 1 – Monitor
```bash
mkdir -p /tmp/jpg
mkfifo /tmp/in.ts

ffmpeg -y -f lavfi -i color=size=640x480:rate=1:color=black \
-frames:v 1 -update 1 /tmp/jpg/latest.jpg

ffplay udp://@:2000
```
This window displays the received MPEG-TS stream.

### Window 2 – DVB-S2 Transmitter

Start the transmitter (Auto Beacon).
```bash
cd ~/dvb-s
./autobeacon.sh 438000000
```
### Window 3 – DVB-S2 Software Receiver

Start the receiver only after the transmitter begins emitting the DVB-S2 signal.
```bash
cd ~/dvb-s
./RF_UDP_dvbs2_rx.py
```

Important:
If transmission stops, terminate the receiver using CTRL-C.

### Window 4 – TS Forwarding

Start this only after TS packets appear in Window 3.
Wait about 30 seconds for receiver synchronization.
```bash
cd ~/dvb-s

sleep 30

ffmpeg -y -re -re \
-i tmp.ts \
-c copy \
-f mpegts \
udp://127.0.0.1:2000
```
Important:
If reception stops, terminate using CTRL-C.

### System Flow
```bash
DVB-S2 Transmitter
        ↓
        RF
        ↓
RTL-SDR
        ↓
GNU Radio DVB-S2 Receiver
        ↓
tmp.ts
        ↓
ffmpeg
        ↓
UDP Stream
        ↓
ffplay
```

### Key Rule
```bash
Start TX first
→ Start RX
→ Wait for TS
→ Start ffmpeg
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
