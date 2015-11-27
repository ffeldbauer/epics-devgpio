#!/bin/bash

if [ $# -ne 1 ] ; then
  echo "Invalid number of arguments: $#"
  echo "Usage: $0 <path>"
  exit -1
fi

chgrp -R gpio $1
chmod -R g+w $1

## this should be moved to rc.local
chgrp gpio /sys/class/gpio/export
chgrp gpio /sys/class/gpio/unexport
chmod g+w /sys/class/gpio/export
chmod g+w /sys/class/gpio/unexport

