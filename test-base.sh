#!/bin/bash

rm -rf tmp
mkdir tmp

gcc -v

cp -r test/base tmp/base
cp fari tmp/base/fari
cd tmp/base
./fari
cd ../..
./tmp/base/f
e_code=$?
cat ./tmp/base/logs.json
if [ $e_code != 3 ]; then
  printf "file1 f not executed correctly"
  exit 2
else
  if [ ! -f "./tmp/base/logs.json" ]; then
    printf "logs.json not found !!"
    exit 2
  else
    printf "OK!"
  fi
fi

sleep 1


