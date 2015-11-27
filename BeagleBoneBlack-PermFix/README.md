Install udev File
=================

The installation of the udev file will change the file permissions of /sys/class/gpio and /sys/devices/virtual/gpio to root:gpio.
You will need to create the gpio group. See below.

As root:

 1. `# cp BeagleBoneBlack-PermFix/80-gpio.rules /etc/udev/rules.d/`
 2. `# cp BeagleBoneBlack-PermFix/gpioudev.sh /bin/`
 3. `# groupadd -r gpio` # Creates a group named `gpio`.
 4. `# usermod -a -G gpio <your username>` # Add yourself to the `gpio` group.
 5. `# reboot` # Reboot the Beagle Bone

After the Beagle Bone comes back up you should see that both directories mentiond above should have owner and group `root:gpio`.

