# DENSO IPC8 driver Makefile
#
# This file is responsible to compile and load the kernel module in 2 ways:
#  - Without DKMS (Just compile and insert the kernel module with insmod and rmmod)
#  - With DKMS (Add, build and install, using DKMS, the kernel module using "dkms add", "dkms build" and "dkms install")
#
# Copyright (C) 2017  DENSO WAVE INCORPORATED <denso_robot_soft@denso-wave.co.jp>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Module name
MODULE_NAME = denso_ipc8

# Module filename
MODULE_NAME_KO = $(MODULE_NAME).ko

# Ensure $M variable is set
ifeq ($(M),)
  # When calling make, the $M variable is the path in the kernel
  # source code (/usr/src/linux-*)
  # For other action we use the current path
  M=.
endif

# Get the version from the VERSION file
MODULE_VERSION = $(shell cat $M/VERSION)

# Objects to compile when calling make
obj-m += denso_ipc8.o

# Where the module source will be placed before to call dkms add, build and install
DKMS_ROOT_PATH = /usr/src/$(MODULE_NAME)-$(MODULE_VERSION)

# Check if the module is loaded or not
MODPROBE_OUTPUT=$(shell lsmod | grep $(MODULE_NAME))

# Source files
CFILES = ipc8.c \
	ipc8_fan.c \
	ipc8_fram.c \
	ipc8_io.c \
	ipc8_ps_comm.c \
	ipc8_fpga.c \
	ipc8_pm.c

# Object files
denso_ipc8-objs := $(CFILES:.c=.o)

# Compilation flags
EXTRA_CFLAGS = -DDRIVER_VERSION=\"$(MODULE_VERSION)\"

# Ensure $KVER variable is set
ifeq ($(KVER),)
  KVER = $(shell uname -r)
endif

# make
all:
	$(MAKE) -C /lib/modules/$(KVER)/build M=$(PWD) modules

# make clean
clean:
	$(MAKE) -C /lib/modules/$(KVER)/build M=$(PWD) clean

# make install (While developing, without using DKMS)
install:
	@echo "Installing DENSO IPC8 driver module..."
	@insmod $(MODULE_NAME_KO)

# make uninstall (While developing, without using DKMS)
uninstall:
	@echo "Uninstalling DENSO IPC8 driver module..."
	@rmmod $(MODULE_NAME_KO)

# make reinstall
reinstall:
	$(MAKE) uninstall
	$(MAKE) install

# ~~~~ DKMS actions ~~~~
# make dkms (Install the module using DKMS)
dkms:
	@echo "Installing DENSO IPC8 driver module version $(MODULE_VERSION)..."
	@sed -i -e '/^PACKAGE_VERSION=/ s/=.*/=\"$(MODULE_VERSION)\"/' dkms.conf
	@mkdir $(DKMS_ROOT_PATH)
	@cp `pwd`/dkms.conf $(DKMS_ROOT_PATH)
	@cp `pwd`/*.c $(DKMS_ROOT_PATH)
	@cp `pwd`/*.h $(DKMS_ROOT_PATH)
	@cp `pwd`/Makefile $(DKMS_ROOT_PATH)
	@cp `pwd`/VERSION $(DKMS_ROOT_PATH)
	@dkms add -m $(MODULE_NAME) -v $(MODULE_VERSION)
	@dkms build -m $(MODULE_NAME) -v $(MODULE_VERSION)
	@dkms install --force -m $(MODULE_NAME) -v $(MODULE_VERSION)
	@echo "Loading DENSO IPC8 driver module..."
	@modprobe $(MODULE_NAME)

# make cleandkms (Remove the module from DKMS)
cleandkms:
	@if [ ! -z "$(MODPROBE_OUTPUT)" ]; then \
		echo "Uninstalling DENSO IPC8 driver module...";\
		rmmod $(MODULE_NAME);\
	fi
	@echo "Removing DENSO IPC8 driver module..."
	@dkms remove -m $(MODULE_NAME) -v $(MODULE_VERSION) --all
	@rm -rf $(DKMS_ROOT_PATH)

# make rebuilddkms
rebuilddkms:
	$(MAKE) cleandkms; \
	$(MAKE) dkms
# ~~~~~~~~~~~~~~~~~~~~~~
