#!/bin/sh

set -e

apt update
apt upgrade
apt install build-essential
apt install libusb-1.0-0-dev

exit 0
