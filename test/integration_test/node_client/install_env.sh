#!/usr/bin/env bash

PYTHON_INTER="python3.7"
VENV_NAME="GRPC_VENV"

sudo apt install -y python3-pip virtualenv
virtualenv -p ${PYTHON_INTER} ${VENV_NAME}
# not working in script
source "${VENV_NAME}/bin/activate"
python -m pip install --upgrade pip
python -m pip install grpcio
python -m pip install grpcio-tools
