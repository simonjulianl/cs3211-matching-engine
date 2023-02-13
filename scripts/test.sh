#!/bin/bash

make clean
make
for i in tests/*; do
  echo -e "\nTEST CASE ${i}"
  ./grader ./engine < ${i}
done