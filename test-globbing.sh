#!/bin/bash

rm -rf tmp
mkdir tmp

cp -r test/glob tmp/glob
cp fari tmp/glob/fari
cd tmp/glob
./fari
cd ../..
./tmp/glob/f
e_code=$?
cat ./tmp/glob/logs.json
if [ $e_code != 3 ]; then
  printf " f not executed correctly"
  exit 2
else
  if [ ! -f "./tmp/glob/logs.json" ]; then
    printf "logs.json not found !!"
    exit 2
  else
    printf "OK!"
  fi
fi

sleep 1


