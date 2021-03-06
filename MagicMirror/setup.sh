#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No color
GREEN='\033[0;32m'
Blue='\033[0;34m'
PURPLE='\033[0;35m'
YELLOW='\033[1;33m'
ORANGE='\033[0;33m'
BOLD = '\033[1m'

# Make system new and shiny
printf "${GREEN} Checking requirements ...\n${NC}"

raspi=$(raspi-config nonint get_pi_type)
node_installed=$(which node)
node_version=$($node_installed -v)
npm_version=$(npm -v)

if [ $raspi -ge 3 ]; then 
    printf "${GREEN} Raspberry Pi $raspi detected!\n${NC}"
else
    printf "${RED} You are using uncompatible version of Pi. Please consider using pi3 or higher.\n${NC}"
    exit 1
fi

if [ -x "$(command -v node)" ]; then
    printf "${GREEN} NodeJS is installed! Found version $node_version.\n${NC}"
else
    printf "${RED} NodeJS is not installed! You must install NodeJS and npm to make it work.\n${NC}"
    exit 1
fi

if [ -x "$(command -v npm)" ]; then
    printf "${GREEN} npm is installed! Found version $npm_version.\n${NC}"
else
    printf "${RED} npm is not installed! You must install NodeJS and npm to make it work.\n${NC}"
    exit 1
fi

printf "${GREEN} Going to home directory...\n${NC}"
printf "${GREEN} Updating the system...\n${NC}"
cd ~
sudo apt update && sudo apt upgrade

printf "${GREEN} System updated!\n${NC}"


printf "${GREEN} Cloning MagicMirror...\n${NC}"
# Clone and build MagicMirror
git clone https://github.com/MichMich/MagicMirror
cd MagicMirror
npm install --only=prod --omit=dev

printf "${GREEN} Getting default config... \n${NC}"
# Add default config file
wget -O config/config.js https://raw.githubusercontent.com/MasterPi-2124/SmartMirror/master/MagicMirror/config.js


# Install Google Assistant
cd modules
printf "${GREEN} Cloning EXT-Detector... \n${NC}"
git clone https://github.com/bugsounet/EXT-Detector

printf "${GREEN} Cloning MMM-GoogleAssistant... \n${NC}"
git clone https://github.com/bugsounet/MMM-GoogleAssistant

printf "${GREEN} Cloning Gateway... \n${NC}"
git clone https://github.com/bugsounet/Gateway

printf "${GREEN} Cloning EXT-Alert... \n${NC}"
git clone https://github.com/bugsounet/EXT-Alert

cd EXT-Detector && npm install
cd ../Gateway && npm install
cd ../EXT-Alert && npm install
cd ../MMM-GoogleAssistant && wget https://raw.githubusercontent.com/MasterPi-2124/SmartMirror/master/MagicMirror/credentials.json && npm install

printf "${GREEN} Getting token for Google Assistant module..."
npm run token
printf "${GREEN} MagicMirror is installed successfully! You can run it via \"npm run start\" command. \n${NC}"
