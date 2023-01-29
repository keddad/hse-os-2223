#!/bin/bash

i=0

while [ $i -le $1 ]
do
  echo $(date)
  sleep 1
  ((i++))
done
