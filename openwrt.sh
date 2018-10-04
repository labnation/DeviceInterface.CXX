#Source this file, don't run it
export TARGET=OPENWRT
export OPENWRT_PATH=~/labnation/openwrt/openwrt/
export STAGING_DIR=$OPENWRT_PATH/staging_dir
export CROSS_COMPILE=${STAGING_DIR}/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-musl-
export PREFIX=`realpath $STAGING_DIR/target-*musl*/usr/`
