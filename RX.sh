#!/usr/bin/env bash
set -euo pipefail

# 受像機の作業ディレクトリを固定（相対パス事故防止）
cd /home/shinji-y/dvb-s

echo "===== DVB-S2 RX START ====="

# 既存の多重起動を確実に止める（ゾンビ対策）
pkill -TERM -f "/home/shinji-y/dvb-s/RF_UDP_dvbs2_rx.py" 2>/dev/null || true
sleep 1
pkill -KILL -f "/home/shinji-y/dvb-s/RF_UDP_dvbs2_rx.py" 2>/dev/null || true

# RX を起動（バックグラウンド）
python3 /home/shinji-y/dvb-s/RF_UDP_dvbs2_rx.py &
RX_PID=$!

echo "RX PID = ${RX_PID}"
echo "Press Ctrl+C to stop."

# Ctrl+C / kill を受けたら確実に回収して終了
cleanup () {
  echo ""
  echo "===== RX STOP ====="
  kill -TERM "${RX_PID}" 2>/dev/null || true
  sleep 1
  kill -KILL "${RX_PID}" 2>/dev/null || true
  wait "${RX_PID}" 2>/dev/null || true
  echo "done."
}
trap cleanup INT TERM EXIT

# ここで待つ（wait しないとゾンビ化しやすい）
wait "${RX_PID}"
