#!/usr/bin/env bash
set -euo pipefail

### CONFIG
FIFO="/tmp/in.ts"
CAM="/dev/video0"
FREQ="${1:-1295000000}"
SIZE="640x480"
FPS="30"
INPUT_FMT="yuyv422"

VBIT="120k"
ABIT="24k"
SR="22050"
MUXRATE="333k"

TX_SECONDS=180
SLEEP_SECONDS=60

DVBS2="./RF_FIFO_dvbs2_tx_rx.py"

log(){ echo "[$(date '+%F %T')] $*"; }

cleanup_procs() {
  [[ -n "${FFMPEG_PID:-}" ]] && kill "${FFMPEG_PID}" 2>/dev/null || true
  [[ -n "${DUMMY_PID:-}"  ]] && kill "${DUMMY_PID}" 2>/dev/null || true
  [[ -n "${DVBS2_PID:-}"  ]] && kill "${DVBS2_PID}" 2>/dev/null || true

  pkill -f "RF_FIFO_dvbs2_tx_rx.py" 2>/dev/null || true
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
  log "start dvbs2: ${DVBS2} -z ${FREQ}"
  "${DVBS2}" -z "${FREQ}" > /tmp/dvbs2.log 2>&1 &  DVBS2_PID=$!
  sleep 1

  # EOF 回避用ダミー writer
  log "start dummy-writer"
  python3 -c "import time; f=open('${FIFO}','wb', buffering=0); time.sleep(10**9)" \
    > /tmp/dummy_writer.log 2>&1 &
  DUMMY_PID=$!
  sleep 1

  # ffmpeg（3分で停止）
  log "start ffmpeg for ${TX_SECONDS}s : ${CAM} -> TS -> ${FIFO}"
  timeout "${TX_SECONDS}" ffmpeg -hide_banner -y -re -loglevel warning -re \
  -stream_loop -1 \
  -i ~/dvb-s/data/out_800x480_av_mp2.ts \
  -c copy \
  -f mpegts \
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