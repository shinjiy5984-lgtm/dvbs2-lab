#!/bin/bash
set -u

echo "[RX] Cleaning previous processes..."

pkill -f RF_UDP_dvbs2_rx.py 2>/dev/null
pkill -f ffplay 2>/dev/null
pkill -f ffmpeg 2>/dev/null

sleep 1

RX_HZ="$1"
MODE="$2"
SR="$3"

echo "[RX] Start: freq=$RX_HZ mode=$MODE sr=$SR"

rm -f /tmp/rx_stdout.log
rm -f /tmp/ffmpeg_rx.log
rm -f /tmp/ffplay_rx.log

# 窓1
mkdir -p /tmp/jpg
rm -f /tmp/in.ts
mkfifo /tmp/in.ts

ffmpeg -y -f lavfi -i color=size=640x480:rate=1:color=black \
    -frames:v 1 -update 1 /tmp/jpg/latest.jpg

ffplay udp://@:2000 > /tmp/ffplay_rx.log 2>&1 &
FFPLAY_PID=$!
echo $FFPLAY_PID > /tmp/ffplay_rx.pid

# 窓3
(
    cd ~/dvb-s || exit 1
    ./RF_UDP_dvbs2_rx.py -g "$RX_HZ" -m "$MODE" -s "$SR" -o 4
) > /tmp/rx_stdout.log 2>&1 &
RX_PID=$!
echo $RX_PID > /tmp/rx_pid

new="0"
old="0"
count="0"

while true
do
    sleep 2
    new=$(wc -c < /tmp/rx_stdout.log 2>/dev/null || echo 0)

    if [ "$new" -gt "$old" ]; then
        echo "[RX] log growing"
        count=$((count + 1))
    else
        count=0
    fi

    if [ "$count" -ge 2 ]; then
        echo "[RX] active RX detected"
        sleep 20
        echo "[RX] start ffmpeg"

        (
            cd ~/dvb-s || exit 1
            ffmpeg -re -i tmp.ts -c copy -muxdelay 4 -muxpreload 4 \
                -f mpegts udp://127.0.0.1:2000
        ) > /tmp/ffmpeg_rx.log 2>&1 &

        FFMPEG_PID=$!
        echo $FFMPEG_PID > /tmp/ffmpeg_rx.pid

        break
    fi

    old=$new
done

echo "[RX] Started."
echo "[RX] ffplay PID : $FFPLAY_PID"
echo "[RX] rx PID     : $RX_PID"
echo "[RX] ffmpeg PID : $FFMPEG_PID"
echo "[RX] Frequency  : $RX_HZ"
echo "[RX] Symbol rate: $SR"
echo "[RX] Mode       : $MODE"
