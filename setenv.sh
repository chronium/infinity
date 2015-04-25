# Yes, this is probably witchcraft. I should probably 
# use some proper tools to generate a nice makefile 
# and everything, but right now this will suffice

export INF_CC="/home/grunt/opt/cross/i686-elf/bin"
export INF_CC_LIB="/home/grunt/opt/i686-infinity/lib"
export INF_CC_INC="/home/grunt/opt/i686-infinity/include"
export INF_BIN_PATH="$(pwd)/kernel/bin/initrd/bin"
export INF_SBIN_PATH="$(pwd)/kernel/bin/initrd/sbin"
export PATH="$PATH:$INF_CC"
