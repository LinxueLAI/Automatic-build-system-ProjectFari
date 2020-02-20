#!/bin/bash

rm -rf tmp
mkdir tmp


cp -r test/java tmp/java
cp fari tmp/java/fari
cd tmp/java
./fari

if [ ! -f "HelloWorld.class" ]; then
	e_code=1
else
	e_code=0
fi

echo $e_code


cat logs.json

cd ../..



if [ $e_code != 0 ]; then
  printf " java class file was not generated correctly"
  exit 2
else
  printf "OK!"
    
fi



