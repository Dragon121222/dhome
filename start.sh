#!/bin/bash

pkill -f wakeWord.py
pkill -f stt.py

if [[ "$(basename "$PWD")" == "build" ]]; then
    make
    sudo make install
else
    mkdir -p build 
    cp start.sh build/start.sh
    cd build 
    cmake .. make 
    sudo make install
fi

sudo systemctl daemon-reload
sudo systemctl restart dhome
sudo systemctl status dhome