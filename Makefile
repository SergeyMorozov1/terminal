# file      Makefile
# copyright Copyright (c) 2012 Toradex AG
#           [Software License Agreement]
# author    $Author$
# version   $Rev$
# date      $Date$
# brief     a simple makefile to (cross) compile.
#           uses the openembedded provided sysroot and toolchain
# target    linux on Colibri imx6
# caveats   -

##############################################################################
# Setup your project settings
##############################################################################

# Create resources file
# vi terminal.gresource.xml
# /usr/lib/x86_64-linux-gnu/glib-2.0/glib-compile-resources --target=resources.c --generate-source terminal.gresource.xml

# create .pot file for multilingual support
# xgettext --from-code=UTF-8 -o terminal.pot -kN_ -k_ -c -C -f strings.txt
# cp terminal.pot ru.po
# msgfmt -vo ru.mo ru.po
# sudo cp ./ru.mo /usr/share/locale/ru/LC_MESSAGES/terminal.mo
# scp /home/dmitriy/workspace/terminal/po/ru.mo root@10.0.0.191:/usr/lib/locale/ru_RU/LC_MESSAGES/terminal.mo
# scp /home/dmitriy/workspace/terminal/po/kk.mo root@10.0.0.191:/usr/lib/locale/kk_KZ/LC_MESSAGES/terminal.mo
# scp /home/dmitriy/workspace/terminal/po/en.mo root@10.0.0.191:/usr/lib/locale/en_US/LC_MESSAGES/terminal.mo

# create update tar
# cd /home/dmitriy/workspace/terminal/
# tar -zcvf terminal.tgz terminal

# Set the input source files, the binary name and used libraries to link
SRCS = terminal.c resources.c nxjson.c
PROG := terminal
LIBS = 

# Set flags to the compiler and linker
#CFLAGS += -O0 -g -Wall `$(PKG-CONFIG) --cflags gtk+-2.0 gthread-2.0` $(ARCH_CFLAGS) 
CFLAGS += -g -Wall `$(PKG-CONFIG) --cflags gtk+-2.0 gthread-2.0 gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0` $(ARCH_CFLAGS)
LDFLAGS += `$(PKG-CONFIG) --libs gtk+-2.0 gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0` -lcurl -luuid

##############################################################################
# Setup your build environment
##############################################################################

# Set the path to the oe built sysroot and
# Set the prefix for the cross compiler
OECORE_NATIVE_SYSROOT ?= $(HOME)/oe-core/build/out-glibc/sysroots/x86_64-linux/
OECORE_TARGET_SYSROOT ?= $(HOME)/oe-core/build/out-glibc/sysroots/colibri-imx6/
CROSS_COMPILE ?= $(OECORE_NATIVE_SYSROOT)usr/bin/arm-angstrom-linux-gnueabi/arm-angstrom-linux-gnueabi-

##############################################################################
# The rest of the Makefile usually needs no change
##############################################################################

# Set differencies between native and cross compilation
ifneq ($(strip $(CROSS_COMPILE)),)
  CFLAGS += -O3 
  LDFLAGS += -L$(OECORE_TARGET_SYSROOT)usr/lib -Wl,-rpath-link,$(OECORE_TARGET_SYSROOT)usr/lib -L$(OECORE_TARGET_SYSROOT)lib -Wl,-rpath-link,$(OECORE_TARGET_SYSROOT)lib 
#  LDFLAGS += -L/home/dmitriy/workspace/terminal -lMsprintsdk -lstdc++ 
  ARCH_CFLAGS = --sysroot=$(OECORE_TARGET_SYSROOT) -march=armv7-a -fno-tree-vectorize -mthumb-interwork -mfloat-abi=hard -mtune=cortex-a9
  BIN_POSTFIX =
  PKG-CONFIG = export PKG_CONFIG_SYSROOT_DIR=$(OECORE_TARGET_SYSROOT); \
               export PKG_CONFIG_PATH=$(OECORE_TARGET_SYSROOT)usr/lib/pkgconfig/; \
               $(OECORE_NATIVE_SYSROOT)usr/bin/pkg-config
else
# Native compile
#  LDFLAGS += -L/home/dmitriy/workspace/terminal -lMsprintsdk_x86 -lstdc++
  CFLAGS += -O0 
  PKG-CONFIG = pkg-config
  ARCH_CFLAGS = 
# Append .x86 to the object files and binaries, so that native and cross builds can live side by side
  BIN_POSTFIX = .x86
endif

# Toolchain binaries
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
RM = rm -f

# Sets the output filename and object files
PROG := $(PROG)$(BIN_POSTFIX)
OBJS = $(SRCS:.c=$(BIN_POSTFIX).o)
DEPS = $(OBJS:.o=.o.d)

# pull in dependency info for *existing* .o files
-include $(DEPS)

all: $(PROG)

$(PROG): $(OBJS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	#$(STRIP) $@ 

%$(BIN_POSTFIX).o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
	$(CC) -MM $(CFLAGS) $< > $@.d

clean:
	$(RM) $(OBJS) $(PROG) $(DEPS)

.PHONY: all clean
