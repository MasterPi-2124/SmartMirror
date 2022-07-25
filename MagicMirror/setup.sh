#!/bin/bash

# Make system new and shiny
cd ~
sudo apt update && sudo apt upgrade

# Clone and build MagicMirror
git clone https://github.com/MichMich/MagicMirror
cd MagicMirror
npm install --only=prod --omit=dev

# Add default config file
wget -O config/config.js https://raw.githubusercontent.com/MasterPi-2124/SmartMirror/master/config/config.js

# Install Google Assistant
cd modules
git clone https://github.com/bugsounet/EXT-Detector
git clone https://github.com/bugsounet/MMM-GoogleAssistant
git clone https://github.com/bugsounet/Gateway
git clone https://github.com/bugsounet/EXT-Alert
cd EXT-Detector && npm install
cd ../Gateway && npm install
cd ../EXT-Alert && npm install
cd ../MMM-GoogleAssistant && npm install



# Run only time hahaha
rm $FILE
