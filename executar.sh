#!/bin/bash
while IFS= read -r comando
do
  echo "$comando"
  echo "$comando" | ./shell
done < comandos.txt
