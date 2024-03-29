#!/bin/bash

if [ ! -d env ]; then
    python3 -m venv env
fi

source env/bin/activate

pip install -r requirements.txt

if [ ! -f matrices ]; then
    ./gen-matrices.py matrices 512
fi
