#!/bin/sh

# https://stackoverflow.com/questions/18951276/echoing-date-automated-git-commit

cd /mnt/hdd/Capture_Edit/Shotcut/ShotcutProjectCollector/ && \
git add . && \
git commit -m "one-click commit: $(date)"  \
