#!/bin/bash
set -e

echo "Installing development tools"
pip install -r sw/tools/requirements.txt

echo "Installing documentation requirements..."
pip install -r sw/docs/requirements.txt
