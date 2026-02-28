# dvbs2-lab
Raspberry Pi DVB-S2 experiment

# dvbs2-lab

Raspberry Pi 5 (64-bit) DVB-S2 beacon experiment using GNU Radio 3.10 + gr-dvbs2.  
Headless-first design for long-duration stability testing.

---

## System Overview

**Hardware**
- Raspberry Pi 5 (64-bit OS)
- USB camera (optional for live input)
- SDR TX chain (external)

**Software**
- GNU Radio 3.10
- gr-dvbs2
- ffmpeg
- Bash automation
- Headless operation (no desktop dependency)

---

## Design Principles

- Headless-first operation
- Minimal CPU load
- No memory growth
- No swap usage
- Automatic process cleanup
- FIFO-based TS injection
- 1 fps JPEG monitoring only (no full video preview)

---

## 24-Hour Endurance Test Result

Test duration: 24 hours continuous operation

- Temperature stable (~35°C)
- No swap usage
- No memory growth observed
- No crash
- No FIFO deadlock
- No orphan processes

TX monitoring reduced to 1 fps (JPEG extraction only) to minimize CPU load.

System confirmed stable for long-duration beacon operation.

---

## Operation Windows

### Window 1 – Preparation

```bash
mkdir /tmp/jpg
mkfifo /tmp/in.ts
ffmpeg -y -f lavfi -i color=size=640x480:rate=1:color=black \
  -frames:v 1 -update 1 /tmp/jpg/latest.jpg
  
Window 2 – Monitor
./dvb-s/monitor.sh
Displays /tmp/jpg/latest.jpg at 1 fps using feh with watchdog restart.  

Window 3 – Beacon
cd dvb-s
./autobeacon.sh

Features:
    • FIFO recreated every cycle
    • dvbs2 process auto-restart
    • Dummy writer to prevent FIFO EOF
    • 180 sec TX + 60 sec idle loop
    • Full cleanup on SIGINT/SIGTERM
    • Power-cycle tolerant design

Beacon Cycle Structure
prepare FIFO
start dvbs2
start dummy writer
start ffmpeg (TS injection + 1fps JPEG monitor)
wait TX duration
cleanup processes
sleep
repeat

Power-Cycle Testing (Next Phase)
Planned tests:
    1. Software reboot repetition
    2. Hard power cut during idle
    3. Hard power cut during active TX
    4. FIFO integrity verification after reboot
Validation checklist:
    • /tmp/in.ts recreated as FIFO
    • No orphan dvbs2/ffmpeg processes
    • JPEG monitoring resumes
    • Log regeneration confirmed

Repository Structure
scripts/    beacon and monitoring scripts
docs/       operational notes
configs/    safe public configuration examples

Status

✔ 24-hour endurance test completed
→ Ready for power-cycle validation

Notes

This repository focuses on operational stability of DVB-S2 beacon systems under constrained hardware environments.

Designed for reproducible headless operation.


---

このREADMEは：

- 技術的に十分具体的
- 余計な個人情報なし
- 公開しても安全
- エンジニアが読めばすぐ理解できる構成

になっています。

---

もしよければ次に：

- 「学術風に格上げ版」
- 「シリコンバレー仕様の硬質版」
- 「論文形式アブストラクト付き版」

どれにしますか？
