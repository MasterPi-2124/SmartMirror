#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No color
GREEN='\033[0;32m'
Blue='\033[0;34m'
PURPLE='\033[0;35m'
YELLOW='\033[1;33m'
ORANGE='\033[0;33m'
BOLD='\033[1m'
NODE_TESTED="v16.9.1"
NPM_TESTED="V7.11.2"

# Make system new and shiny
printf "${YELLOW}Checking requirements ...\n${NC}"

raspi=$(raspi-config nonint get_pi_type)
arm=$(uname -m)

function verlte() {  [ "$1" = "`echo -e "$1\n$2" | sort -V | head -n1`" ];}
function verlt() { [ "$1" = "$2" ] && return 1 || verlte $1 $2 ;}

if [ 0 == 1 ]; then
	if [ "$arm" != "armv7l" ]; then
        printf "${RED}You are using uncompatible version of Pi. Please consider using pi3 or higher.\n${NC}"
        exit 1
	elif [ $raspi -ge 3 ]; then 
        printf "${GREEN}Raspberry Pi $raspi detected!\n${NC}"
    fi
fi

if [ -x "$(command -v node)" ]; then
    node_version=$(node -v)
    if verlt $node_version $NODE_TESTED; then
        printf "${RED}NodeJS version is too old! MagicMirror only run on NodeJS >= $NODE_TESTED.\n${NC}"
        exit 1
    else
        printf "${GREEN}NodeJS is installed! Found version $node_version.\n${NC}"
    fi
else
    printf "${RED}NodeJS is not installed! You must install NodeJS and NPM to make it work.\n${NC}"
    exit 1
fi

if [ -x "$(command -v npm)" ]; then
    npm_version='V'$(npm -v)
    if verlte $npm_version $NPM_TESTED; then
        printf "${RED}NPM version is too old! MagicMirror only run on NPM >= $NPM_TESTED.\n${NC}"
        exit 1
    else
        printf "${GREEN}NPM is installed! Found version $npm_version.\n${NC}"
    fi
else
    printf "${RED}NPM is not installed! You must install NodeJS and NPM to make it work.\n${NC}"
    exit 1
fi

printf "${YELLOW}Going to home directory...\n${NC}"
printf "${YELLOW}Updating the system...\n${NC}"
cd ~
sudo apt update && sudo apt upgrade

printf "${GREEN}System updated!\n${NC}"


printf "${YELLOW}Cloning MagicMirror...\n${NC}"
# Clone and build MagicMirror
git clone https://github.com/MichMich/MagicMirror
cd MagicMirror
npm install --only=prod --omit=dev

install_status=$?
if [ $install_status -eq 0 ]; then
    printf "${GREEN}MagicMirror install completed!\n${NC}"
else 
    printf "${RED}MagicMirror install failed with errors!\n${NC}"
    exit 1
fi

printf "${YELLOW}Getting default config... \n${NC}"
# Add default config file
wget -O config/config.js https://raw.githubusercontent.com/MasterPi-2124/SmartMirror/master/MagicMirror/config.js
printf "${GREEN}Configuration saved successfully at config/config.js.\n${NC}"

# Install Google Assistant
cd modules
printf "${YELLOW}Cloning EXT-Detector... \n${NC}"
git clone https://github.com/bugsounet/EXT-Detector

printf "${YELLOW}Cloning MMM-GoogleAssistant... \n${NC}"
git clone https://github.com/bugsounet/MMM-GoogleAssistant

printf "${YELLOW}Cloning Gateway... \n${NC}"
git clone https://github.com/bugsounet/Gateway

printf "${YELLOW}Cloning EXT-Alert... \n${NC}"
git clone https://github.com/bugsounet/EXT-Alert

printf "${YELLOW}Cloning MMM-OpenCVGestures... \n${NC}"
git clone https://github.com/MasterPi-2124/MMM-OpenCVGestures

printf "${GREEN}Modules cloned successfully!\n${NC}"

printf "${YELLOW}Installing EXT-Detector... \n${NC}"
cd EXT-Detector && npm install
install_status=$?
if [ $install_status -eq 0 ]; then
    printf "${GREEN}EXT-Detector install completed!\n${NC}"
else 
    printf "${RED}EXT-Detector install failed with errors!\n${NC}"
    exit 1
fi

printf "${YELLOW}Installing Gateway... \n${NC}"
cd ../Gateway && npm install
install_status=$?
if [ $install_status -eq 0 ]; then
    printf "${GREEN}Gateway install completed!\n${NC}"
else 
    printf "${RED}Gateway install failed with errors!\n${NC}"
    exit 1
fi

printf "${YELLOW}Installing EXT-Alert... \n${NC}"
cd ../EXT-Alert && npm install
install_status=$?
if [ $install_status -eq 0 ]; then
    printf "${GREEN}EXT-Alert install completed!\n${NC}"
else 
    printf "${RED}EXT-Alert install failed with errors!\n${NC}"
    exit 1
fi

printf "${YELLOW}Installing MMM-GoogleAssistant... \n${NC}"
cd ../MMM-GoogleAssistant
printf "${YELLOW}Installing credentials... \n${NC}"
wget https://raw.githubusercontent.com/MasterPi-2124/SmartMirror/master/MagicMirror/credentials.json
printf "${GREEN}Credentials saved successfully!\n${NC}"
npm install
install_status=$?
if [ $install_status -eq 0 ]; then
    printf "${GREEN}MMM-GoogleAssistant install completed!\n${NC}"
else 
    printf "${RED}MMM-GoogleAssistant install failed with errors!\n${NC}"
    exit 1
fi

printf "${YELLOW}Getting token for Google Assistant module..."
npm run token
printf "\e[92mWe're ready! Run \e[1m\e[97m$rmessage\e[0m\e[92m from the ~/MagicMirror directory to start your MagicMirror.\e[0m"
