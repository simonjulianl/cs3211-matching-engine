#!/bin/bash


TESTCASES=(basic-example1.in basic-example2.in basic-example3.in)

make clean
make
for i in ${!TESTCASES[@]}; do
  echo -e "\nTEST CASE ${i}"
  ./grader ./engine < tests/${TESTCASES[i]}
done