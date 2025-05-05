#!/usr/bin/env bash

default_nonos="NONOSDK221 NONOSDK22x_190313 NONOSDK22x_190703 NONOSDK22x_191024 NONOSDK22x_191105 NONOSDK22x_191122 NONOSDK305"
default_prefix="rom_"

cmd=$1
nonos=${2:-${default_nonos}}
prefix=${3:-${default_prefix}}

symbols="memcmp memcpy memmove strcmp strcpy strlen strncmp strncpy strstr"
re_show=""

OBJCOPY="xtensa-lx106-elf-objcopy"
for s in $symbols ; do
    OBJCOPY+=" --redefine-sym ${s}=${prefix}${s}"
done

AR=xtensa-lx106-elf-ar
NM=xtensa-lx106-elf-nm

for d in $nonos ; do
    mkdir -p ${d}/tmp
    pushd ${d}/tmp

    if [ x$cmd = "xedit" ] ; then
        for a in ../*.a ; do
            ar Dx $a
            for o in *.o ; do
                $OBJCOPY $o
            done
            $AR Dr $a *.o
            rm *.o
        done
    elif [ x$cmd = "xshow" ] ; then
        for a in ../*.a ; do
            ar x $a
        done
        for o in *.o ; do
            objs=$($NM -B $o | grep -P "(${symbols/ /|})" | grep -v $prefix)
            if [ "${#objs}" != "0" ] ; then
                echo $o
                echo $objs
            fi
        done
    fi

    popd
    rm -rf ${d}/tmp
done
