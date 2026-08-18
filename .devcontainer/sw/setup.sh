#!/bin/bash
set -e

echo "Installing local libraries..."
pip install -e sw/tools/python/bike-line-protocol/python-lib
pip install -e sw/tools/python/bike-uds-tool/python-lib

echo "Installing development tools"
pip install -r sw/tools/requirements.txt

echo "Installing documentation requirements..."
pip install -r sw/docs/requirements.txt

echo "Installing debug tools..."
pyocd pack install ATSAMD21E18A
pyocd pack install STM32L431CBYx
