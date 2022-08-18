# epics-devgpio
EPICS device support to control GPIOs on the Single Board computers

# ATTENTION
Version 2 of this device support uses the new V2 ABI for GPIO character device (c.f. `/usr/include/linux/gpio.h`) which was introduced in Kernel 5.x.

If you are using an older kernel version, please use [R1-0-6](https://github.com/ffeldbauer/epics-devgpio/releases/tag/R1-0-6)

# Dependencies
EPICS base 3.14.12.4 (or higher)

# Build
Set path to EPICS base installation in `configure/RELEASE.local` or `../RELEASE.local`
run `make` in the top directory.

# Usage
Version 2 supports single bit (bi/bo) as well as multibit (mbbi/o, mbbi/oDirect) records.

Set the `DTYP` field of your recrod to `devGpio`.
The Syntax for `INP` fields is:
```
@<GPIO1> [GPIO2] [LOW] [FALLING/RISING/BOTH]
```
* (bi records only support one GPIO)
* The `LOW` flag switched the gpio into active low mode
* FALLING/RISING/BOTH enables interrupt on falling, rising, or both edges, respectively

The Syntax for `OUT` fields is:
```
@<GPIO1> [GPIO2] [LOW]
```
* (bi records only support one GPIO)
* The `LOW` flag switched the gpio into active low mode

