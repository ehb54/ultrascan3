#!/bin/bash

qhelpgenerator manual.qhp -o manual.qch
qcollectiongenerator manual.qhcp -o manual.qhc

mv manual.qch manual.qhc ../../bin

