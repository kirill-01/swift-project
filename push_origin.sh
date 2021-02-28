#!/bin/bash
qmake -qt=${1} -r .
make -j clean
make -j distclean
git add .
git commit -m "${2}"
git push
