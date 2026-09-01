#!/bin/bash
echo "Starting setup process"

type=$1
test=$2

read -p "Enter type of test: " type
read -p "Enter specific test file: " test

make clean
make compiler
./compiler ./test_files/$type/$test.cpm
make tree
make symboltable

echo "Done!"