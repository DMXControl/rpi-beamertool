#!/bin/bash

cd /opt/rpi-beamertool
killall rpi-beamertool
sleep 10
./rpi-beamertool --silent
