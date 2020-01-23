#!/bin/bash

git reset --hard
git pull
rm main
make
sudo ./main
