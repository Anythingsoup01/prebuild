#!/bin/bash

./prebuild cmake 3.30

cd build/ && cmake --build . -j 8
