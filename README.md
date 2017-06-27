# C++ Implementation of the SmartScope server

Run this server on any platform to connect to a smartscope over a tcp/ip network connection

## Compiling

The dependencies are libusb, libavahi-client and libavahi-common (and pthread and stdc++)

The PREFIX environment variable defines where the compile is going to look for libraries. This defaults to `/usr` which is fine when compiling on the same machine as you're gonna run it.

If using a cross-compiler, be sure to set PREFIX to the appropriate path with the target machine's libraries, and set the CROSS_COMPILE environment variable to whatever leads up to the c++ compile binary.

### Ubuntu

On ubuntu, install the dependencies using

`sudo apt install libusb-1.0.0-dev libavahi-client-dev libavahi-common-dev`

then run

`make`

### MacOS
On macOS, we don't use avahi but dnssd for the zeroconf feature. There you run

`make DNSSD=1`

### OpenWRT / CrossCompiling

When cross-compiling for OpenWRT, I used the following lines to set the environment

```
export OPENWRT_PATH=~/labnation/openwrt/openwrt/
export STAGING_DIR=$OPENWRT_PATH/staging_dir
export CROSS_COMPILE=${STAGING_DIR}/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-
export PREFIX=`realpath $STAGING_DIR/target-*uClibc*/usr/`
```

then run

`make`

