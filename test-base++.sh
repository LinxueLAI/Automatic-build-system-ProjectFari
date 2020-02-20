#!/bin/bash

rm -rf tmp
mkdir tmp

cp -r test/basep tmp/basep
cp fari tmp/basep/fari
cd tmp/basep
./fari
cd ../..
./tmp/basep/f
e_code=$?
cat ./tmp/basep/logs.json
if [ $e_code != 3 ]; then
  printf " f not executed correctly"
  exit 2
else
  if [ ! -f "./tmp/basep/logs.json" ]; then
    printf "logs.json not found !!"
    exit 2
  else
    printf "OK!"
  fi
fi

sleep 1


