#!/bin/bash

#
# Copyright (C) 2018 Yuvraj Saxena (frap130)
# Copyright (C) 2018 SaMad SegMane (MrSaMaDo)
#

# Script To Compile Kernels

# Colors
ORNG=$'\033[0;33m'
CYN=$'\033[0;36m'
PURP=$'\033[0;35m'
BLINK_RED=$'\033[05;31m'
BLUE=$'\033[01;34m'
BLD=$'\033[1m'
GRN=$'\033[01;32m'
RED=$'\033[01;31m'
RST=$'\033[0m'
YLW=$'\033[01;33m'

function START_SCRIPT() {
if [[ ${BUILDING_ON_CI} = 'y' ]]; then
        args="${*}"
        while [[ ${#} -ge 1 ]]; do
	case "${1}" in
                "-b"|"--build")
                        BUILD=y
                        ;;
		"-c"|"--clean")
			CLEAN=y
			;;
                "-h"|"--help")
                        HELP=y
                        ;;
		*)
			echo "${RED}Opps:Invalid Option${RST}"
                        exit 1;
			;;
		esac
		shift
        done
else
        echo "${GRN} Welcome ${RST}"
        read -rsn1 -p "${PURP}Type h for help (b|c|h):${RST}" O
	case "${O}" in
                "b"|"build")
                        BUILD=y
                        ;;
		"c"|"clean")
			CLEAN=y
			;;
                "h"|"help")
                        HELP=y
                        ;;
		*)
			echo "${RED}Opps:Invalid Option${RST}"
                        exit 1;
			;;
		esac
		shift
fi
}

function BANNER() {
clear
        
        echo "${BLINK_RED}
        

▀█████████▄   ▄█          ▄████████  ▄████████    ▄█   ▄█▄    ▄██████▄     ▄█    █▄     ▄██████▄     ▄████████     ███     
  ███    ███ ███         ███    ███ ███    ███   ███ ▄███▀   ███    ███   ███    ███   ███    ███   ███    ███ ▀█████████▄ 
  ███    ███ ███         ███    ███ ███    █▀    ███▐██▀     ███    █▀    ███    ███   ███    ███   ███    █▀     ▀███▀▀██ 
 ▄███▄▄▄██▀  ███         ███    ███ ███         ▄█████▀     ▄███         ▄███▄▄▄▄███▄▄ ███    ███   ███            ███   ▀ 
▀▀███▀▀▀██▄  ███       ▀███████████ ███        ▀▀█████▄    ▀▀███ ████▄  ▀▀███▀▀▀▀███▀  ███    ███ ▀███████████     ███     
  ███    ██▄ ███         ███    ███ ███    █▄    ███▐██▄     ███    ███   ███    ███   ███    ███          ███     ███     
  ███    ███ ███▌    ▄   ███    ███ ███    ███   ███ ▀███▄   ███    ███   ███    ███   ███    ███    ▄█    ███     ███     
▄█████████▀  █████▄▄██   ███    █▀  ████████▀    ███   ▀█▀   ████████▀    ███    █▀     ▀██████▀   ▄████████▀     ▄████▀   
             ▀                                   ▀                                                                         

                                                    
             
${RST}
"
sleep 2
clear
}

function BINFO() {
        echo "### Edit BINFO If u Want to change Host/User. ###"
        export KBUILD_BUILD_USER=SaMad
        export KBUILD_BUILD_HOST=BlackGHost
}

function TOOLCHAIN() {
if [[ ! -d gtc ]]; then 
    echo "${RED}####################################"
    echo "${CYN}#       TOOLCHAIN NOT FOUND!       #"
    echo "${YLW}####################################"
clear
    sudo rm -rf linaro
    sudo rm -rf gcc/.git
    echo "${YLW}####################################"
    echo "${GRN}#       CLONING TOOLCHAIN          #"
    echo "${YLW}####################################"
    git clone -q https://github.com/svoboda18/android_gcc_6.4.1_arm-eabi gcc
    export ARCH=arm CROSS_COMPILE=${PWD}/gcc/bin/arm-eabi-
    export SUBARCH=arm
else
    export ARCH=arm CROSS_COMPILE=${PWD}/gcc/bin/arm-eabi-
    export SUBARCH=arm
fi
}

function BUILD() {
        mkdir -p out
        echo "${PURP} READING DEFCONFIG..."
        make O=out/ TARGET_ARCH=arm X5_bsp_defconfig | tee -a defconfig.log
        echo "${YLW} BUILDING KERNEL..." 
        make -j6 O=out/ TARGET_ARCH=arm | tee -a Kernel.log
        OIMAGE=out/arch/arm/boot/zImage-dtb
}

function CHECK() {
if [[ ! -e ${OIMAGE} ]];
then
        echo "${RED}############################"
        echo "${RED}#        BUILD ERROR!      #"
        echo "${RED}############################"
        echo "${CYN}#       Uploading Logs     #"
        echo "${RED}############################"
        TRANSFER Kernel.log
else
        echo "${GRN} #####################################"
        echo "${GRN} #                                   #"
        echo "${GRN} #    SUCCESSFULLY BUILDED KERNEL    #"
        echo "${GRN} #                                   #"
        echo "${GRN} #####################################"
        mv ${OIMAGE} boot.img-zImage
        zip -r 3.18.79-zImage.zip boot.img-zImage
        sleep 5
clear
        TRANSFER 3.18.79-zImage.zip
fi
}

function CLEAN() {
clear
        echo "${GRN}#        Cleaning Tree!      ${RST}"
        make clean
        make mrproper
clear
}

function HELP() {
	echo "${ORNG}options:${RST}"
	echo "${CYN}  b, build     Build Kernel${RST}"
	echo "${RST}  c, clean     clean Kernel${RST}"
	echo "${GRN}  h, Help      Options${RST}"
        read junk
}

function TRANSFER() {
        file="$1"
        zipname=$(echo "${file}" | awk -F '/' '{print $NF}')
        destination="$2"
        url=$(curl -# -T "${file}" https://transfer.sh/${destination})
        printf '\n'
        echo "Download $zipname at $url"
}

function FORMAT_TIME() {
        MINS=$(((${1}-${2})/60))
        SECS=$(((${1}-${2})%60))
if [[ ${MINS} -ge 60 ]]; then
        HOURS=$((${MINS}/60))
        MINS=$((${MINS}%60))
fi

if [[ ${HOURS} -eq 1 ]]; then
        TIME_STRING+="1 HOUR, "
elif [[ ${HOURS} -ge 2 ]]; then
        TIME_STRING+="${HOURS} HOURS, "
fi

if [[ ${MINS} -eq 1 ]]; then
        TIME_STRING+="1 MINUTE"
else
        TIME_STRING+="${MINS} MINUTES"
fi

if [[ ${SECS} -eq 1 && -n ${HOURS} ]]; then
        TIME_STRING+=", AND 1 SECOND"
elif [[ ${SECS} -eq 1 && -z ${HOURS} ]]; then
        TIME_STRING+=" AND 1 SECOND"
elif [[ ${SECS} -ne 1 && -n ${HOURS} ]]; then
        TIME_STRING+=", AND ${SECS} SECONDS"
elif [[ ${SECS} -ne 1 && -z ${HOURS} ]]; then
        TIME_STRING+=" AND ${SECS} SECONDS"
fi

        echo ${TIME_STRING}
}

        BANNER

if [[ ${BUILDING_ON_CI} = 'y' ]]; then
        START_SCRIPT "${@}"
else
	START_SCRIPT
fi

if [[ "${BUILD}" = 'y' ]]; then
clear
        START=$(date +"%s")
        TOOLCHAIN
clear
        BINFO
        sleep 0.3
        BUILD
        CHECK
        END=$(date +%s)
        TIME_STRING="$(FORMAT_TIME "${START}" "${END}")"
        echo "${GRN}Completed In: ${TIME_STRING}"
elif [[ "${CLEAN}" = 'y' ]]; then
        CLEAN
else
if [[ "${HELP}" = 'y' ]]; then
clear
        HELP
fi
fi
