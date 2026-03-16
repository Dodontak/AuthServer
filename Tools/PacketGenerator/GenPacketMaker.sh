#!/bin/bash

cd "$(dirname "$0")"
python3 -m PyInstaller   --onefile PacketGenerator.py
mv ./dist/PacketGenerator ./GenPackets
cp ./GenPackets ../../Protocol

rm -rf ./__pycache__
rm -rf ./build
rm -rf ./dist
rm -f ./PacketGenerator.spec