#!/bin/bash
export TARGET=OPENWRT
export OPENWRT_PATH=~/labnation/openwrt/openwrt/
export STAGING_DIR=$OPENWRT_PATH/staging_dir
export CROSS_COMPILE=${STAGING_DIR}/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-
export PREFIX=`realpath $STAGING_DIR/target-*/usr/`
