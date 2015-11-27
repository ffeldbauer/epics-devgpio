# epics-devgpio
EPICS device support to control GPIOs on the BeagleBone Black / Raspberry Pi via the /sys/class/gpio interface

# R/W permissions on GPIOs
On the Beaglebone Black the /sys/class/gpio interface belongs to root:root by default.
To give read/write permissions to a normal user, the udev rule in `BeagleBoneBlack-PermFix`
can be used.

# Dependencies
EPICS base 3.14.12.4 (or higher)

# Build
Change path to EPICS base installation in `configure/RELEASE`
run `make` in the top directory.

# Usage
Refer to the pdf in the doc directory.

