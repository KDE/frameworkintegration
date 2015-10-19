#!/bin/sh

sed -i "s/Oxygen-Sans/Noto Sans/" `kf5-config --path config --locate kdeglobals`
