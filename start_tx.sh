#!/bin/bash
set -u

TX_HZ="$1"
MODE="$2"
SR="$3"
TSFILE="$4"

mkdir -p /tmp/jpg
rm -f /tmp/jpg/latest.jpg
rm -f /tmp/in.ts
mkfifo /tmp/in.ts

ffmpeg -loglevel error -y \
  -f lavfi -i color=size=640x480:rate=1:color=black \
  -frames:v 1 -update 1 /tmp/jpg/latest.jpg

ffplay udp://@:2000 &

./experiment.sh "$TX_HZ" "$MODE" "$SR" "$TSFILE"
