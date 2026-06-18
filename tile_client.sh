#!/bin/bash

APP="./build/client"

SCREEN_W=1920*2
SCREEN_H=1080*2

# When the script exits or is interrupted, kill all children
cleanup() {
    echo "Stopping clients..."
    kill $(jobs -p) 2>/dev/null
}

trap cleanup EXIT INT TERM

COLS=2
ROWS=1

WIN_W=$(( SCREEN_W / COLS ))
WIN_H=$(( SCREEN_H / ROWS ))

for ((i=0; i<COLS*ROWS; i++))
do
    row=$(( i / COLS ))
    col=$(( i % COLS ))

    x=$(( col * WIN_W ))
    y=$(( row * WIN_H ))

    "$APP" "$WIN_W" "$WIN_H" "$x" "$y" &
done

wait