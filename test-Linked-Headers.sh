#!/bin/bash

rm -rf tmp
mkdir tmp

cp -r test/CH tmp/CH
cp fari tmp/CH/fari
cd tmp/CH
./fari
./f
e_code=$?




sleep 5s


touch f1.h
./fari
cat logs.json

B=$(cat logs.json | grep -1 "commands"| tail -1 | sed -e 's/\,//g' -e 's/\"//g')
A=$(cat logs.json | grep -2 "commands"| tail -1 | sed -e 's/\,//g' -e 's/\"//g' -e 's/\ //g')
cd ../..





if [ $e_code != 3 ]; then
  printf " f not executed correctly"
  exit 2
else
  if [ ! -f "./tmp/CH/logs.json" ]; then
    printf "logs.json not found !!"
    exit 2
  else
    if [ "$A" == "]"  ] && [ "$B" == "       gcc -g -c f.c -o f.o" ]; then
    	printf "OK!"
    else
        printf "logs.json doesn't correspond to what was expected !!"
        exit 2
    fi
    
  fi
fi

sleep 1


