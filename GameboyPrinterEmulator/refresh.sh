#!/bin/bash

git reset --hard
git pull origin reply
rm main
make
sudo ./main
