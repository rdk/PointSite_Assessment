#!/bin/bash

# create a python3 environment
conda remove --name selenium --all -y
conda create --name selenium python=3.7 -y
source activate selenium
pip install -r requirements.txt

