#!/usr/bin/env bash
set -u

# ============================================
# feh watchdog (ffmpegはそのまま)
# - /tmp/jpg/latest.jpg を 1秒ごとにリロード表示
# - feh が終了/落ちたら自動で再起動
# - jpg を増やさない（latest.jpg 1枚だけ前提）
# ============================================

mkdir -p /tmp/jpg

# latest.jpg がまだ無い時に feh を起動すると警告がうるさいので待つ
echo "[watchdog] waiting for /tmp/jpg/latest.jpg ..."
while [ ! -s /tmp/jpg/latest.jpg ]; do
  sleep 0.2
done

echo "[watchdog] start feh loop"

while true; do
  # feh を前面実行（落ちたら戻ってくる）
  feh --reload 1 --auto-zoom --title "TX monitor" /tmp/jpg/latest.jpg

  # ここに来た＝feh が終了した（落ちた/閉じた）
  echo "[watchdog] feh exited. restarting in 0.5s..."
  sleep 0.5

  # latest.jpg が消えていたら復帰待ち
  while [ ! -s /tmp/jpg/latest.jpg ]; do
    sleep 0.2
  done
done