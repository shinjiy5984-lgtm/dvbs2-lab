#!/usr/bin/env bash
set -euo pipefail

### CONFIG
FIFO="/tmp/in.ts"
CAM="/dev/video0"

SIZE="640x480"
FPS="30"
INPUT_FMT="yuyv422"

VBIT="120k"
ABIT="24k"
SR="22050"
MUXRATE="333k"

TX_SECONDS=180
SLEEP_SECONDS=60

DVBS2="./dvbs2_tx_rx_webcam_loopback_headless.py"

log(){ echo "[$(date '+%F %T')] $*"; }

cleanup_procs() {
  [[ -n "${FFMPEG_PID:-}" ]] && kill "${FFMPEG_PID}" 2>/dev/null || true
  [[ -n "${DUMMY_PID:-}"  ]] && kill "${DUMMY_PID}" 2>/dev/null || true
  [[ -n "${DVBS2_PID:-}"  ]] && kill "${DVBS2_PID}" 2>/dev/null || true

  pkill -f "dvbs2_tx_rx_webcam_loopback_headless.py" 2>/dev/null || true
  pkill -f "python3 -c.*open\\(\"${FIFO}\""  2>/dev/null || true
  pkill -f "ffmpeg .* -i ${CAM} .* ${FIFO}" 2>/dev/null || true
}

trap 'log "TRAP: cleanup"; cleanup_procs; rm -f "${FIFO}" 2>/dev/null || true; exit 0' INT TERM

while true; do
  log "===== cycle start ====="
  cleanup_procs

  # FIFO 作り直し（dvbs2 起動前）
  rm -f "${FIFO}"
  mkfifo "${FIFO}"
  log "FIFO prepared: ${FIFO}"

  # dvbs2 起動
  log "start dvbs2: ${DVBS2}"
  "${DVBS2}" > /tmp/dvbs2.log 2>&1 &
  DVBS2_PID=$!
  sleep 1

  # EOF 回避用ダミー writer
  log "start dummy-writer"
  python3 -c "import time; f=open('${FIFO}','wb', buffering=0); time.sleep(10**9)" \
    > /tmp/dummy_writer.log 2>&1 &
  DUMMY_PID=$!
  sleep 1

  # ffmpeg（3分で停止）
  log "start ffmpeg for ${TX_SECONDS}s : ${CAM} -> TS -> ${FIFO}"
  timeout "${TX_SECONDS}" ffmpeg -y -re \
  -thread_queue_size 2048 \
  -f v4l2 -input_format yuyv422 -framerate 30 -video_size 640x480 -i /dev/video0 \
  -f lavfi -i sine=frequency=1000:sample_rate=22050 \
  -vf format=yuv420p \
  -c:v libx264 -preset ultrafast -tune zerolatency \
  -profile:v baseline -level 3.0 \
  -x264-params keyint=30:min-keyint=30:scenecut=0 \
  -b:v 220k -maxrate 220k -bufsize 440k \
  -c:a mp2 -ac 1 -ar 22050 -b:a 24k \
  -mpegts_flags resend_headers \
  -muxdelay 0 -muxpreload 0 \
  -t 180 \
  -f mpegts /tmp/in.ts \
  -r 1 -update 1 -q:v 1 /tmp/jpg/latest.jpg \
    > /tmp/ffmpeg.log 2>&1 &
  FFMPEG_PID=$!

  wait "${FFMPEG_PID}" || true
  log "ffmpeg finished (expected)."

  cleanup_procs
  rm -f "${FIFO}" || true

  log "sleep ${SLEEP_SECONDS}s"
  sleep "${SLEEP_SECONDS}"
  log "===== cycle end ====="
done 