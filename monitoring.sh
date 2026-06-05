#!/bin/bash

while true 
do
    clear
    disque=$(df -h / | awk 'NR==2 {print $5}')
    ram=$(free | awk 'NR==2 {print $3/$2 * 100}')
    Cpu=$(top -bn1 | grep "%Cpu" | awk '{print 100 - $8}')
    echo -e "Ressouce\t\t Usage"
    echo -e "----------------------------\n"
    echo -e "Disque  \t\t $disque"
    echo -e "RAM     \t\t $ram%"
    echo -e "CPU     \t\t $Cpu%"
    echo    "-----------------------------"
    sleep 2
done
