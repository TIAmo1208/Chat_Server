#!/bin/bash

abort()
{
    echo >&2 '
*****************************
***** BUILDING  ABORTED *****
*****************************
'
    echo "An error occurred. Exiting..." >&2
    exit 1
}

trap 'abort' 1

Home_Path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

paths=()
names=()

##------------
Log_Path="$Home_Path/Log"
paths+=($Log_Path)
names+=('Log')

threadPool_Path="$Home_Path/threadPool"
paths+=($threadPool_Path)
names+=('threadPool')

server_Path="$Home_Path/server"
paths+=($server_Path)
names+=('server')

cd $FRAMEWORK_PATH/

for i in ${!paths[@]}; do
    echo -e "\033[0;94mStart building ${names[$i]}...\033[0m"
    cd ${paths[$i]}
    rm -rf build
    mkdir build
    cd build/
    echo -e "\033[0;94mBuilding and installing ${names[$i]}...\033[0m"

    cmake ..
    make -j 1  --trace 
    make -j install

    cd ..
    echo -e "\033[0;92m${names[$i]} built.\033[0m"
    rm -rf build
    cd $FRAMEWORK_PATH/
done

trap : 0

echo >&2 '
**************************
***** BUILDING  DONE *****
**************************
'