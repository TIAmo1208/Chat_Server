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

trap 'abort' ERR

Home_Path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

CLEAN="false"

while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
        -c|--clean)
        CLEAN="true"
        shift # past argument
        ;;
    esac
done

paths=()
names=()

##------------

Log_Path="$Home_Path/Log"
paths+=($Log_Path)
names+=('Log')

Config_Path="$Home_Path/Config"
paths+=($Config_Path)
names+=('Config')

ThreadPool_Path="$Home_Path/ThreadPool"
paths+=($ThreadPool_Path)
names+=('ThreadPool')

mysql_Path="$Home_Path/Mysql"
paths+=($mysql_Path)
names+=('Mysql')

server_Path="$Home_Path/Server"
paths+=($server_Path)
names+=('Server')

if [ $CLEAN == "true" ]; then
    rm -rf out
fi

cd $FRAMEWORK_PATH/

for i in ${!paths[@]}; do
    echo -e "\033[0;94mStart building ${names[$i]}...\033[0m"
    cd ${paths[$i]}

    if [ $CLEAN == "true" ]; then
        rm -rf build
    fi

    if [ ! -d "build" ]; then
        mkdir "build"
    fi

    cd build/
    echo -e "\033[0;94mBuilding and installing ${names[$i]}...\033[0m"

    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
    make -j 1  --trace 
    make -j install

    cd ..
    echo -e "\033[0;92m${names[$i]} built.\033[0m"
    cd $FRAMEWORK_PATH/
done

cp $Home_Path/config.cfg $Home_Path/out/bin/

trap : 0

echo >&2 '
**************************
***** BUILDING  DONE *****
**************************
'