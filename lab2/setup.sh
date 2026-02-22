#!/bin/bash

# Lab 2 Setup Script: Environment & Networking
# Run this as root on the DE1-SoC board

echo "--- Starting Networking Setup ---"
# Bring up the ethernet interface
ifup eth0

echo "--- Updating Package Lists ---"
# Update and upgrade existing packages
apt update && apt upgrade -y

echo "--- Installing Development Software ---"
# Install C compiler, make, USB libraries, and utilities
apt install -y gcc make libusb-1.0-0-dev usbutils

echo "--- Installing Text Editors ---"
# Installing nano and vim-tiny as suggested in the lab manual
apt install -y nano vim-tiny

echo "--- Installing Connectivity Tools ---"
# Install SSH client for scp and wget for downloading files
apt install -y openssh-client wget

echo "--- Cleaning Up ---"
# Clear out the local repository of retrieved package files
apt clean

echo "--- Setup Complete ---"
echo "You can now proceed to Part 4: Compiling and Running the Skeleton Files."
