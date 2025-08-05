#!/bin/sh

FTDI_APP=$(pwd)/bin/ConfigureFtdi
RULES_FILE=/etc/udev/rules.d/90-tritech-usb-serial.rules

if [ -f $RULES_FILE ];
then
    rm $RULES_FILE
fi

cat <<EOF > $RULES_FILE
SUBSYSTEM=="tty", ACTION=="add", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6014", ATTRS{product}=="Tritech USB-Serial" SYMLINK+="tritechUsbSerial%n", MODE="0777"
SUBSYSTEM=="tty", ACTION=="remove", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6014", ATTRS{product}=="Tritech USB-Serial" RUN+="rm /dev/tritechUsbSerial%n"
EOF

ftdi_driver_loaded()
{
    ret=`lsmod | grep ftdi_sio`
    if [ ! -z "$ret" ] 
    then
        return 1;
    else
        return 0;
    fi
}

# first of all unload driver to configure RS232/RS485 mode
rmmod ftdi_sio usbserial > /dev/null 2>&1

sleep 1  # Wait 1 second

# confirm ftdi driver is unloaded
ftdi_driver_loaded
if [ $? -eq 1 ]
then
    echo "Failed to unload driver !!";
    echo "Exiting...";
    return 1;
fi


# Now do the configuration
[ -f $FTDI_APP ] && $FTDI_APP "$1"

sleep 5  # Waits 5 seconds

# Reload driver to access /dev/tritechUSBSerial0
#udevadm trigger
#udevadm trigger --action=add && udevadm trigger
/sbin/modprobe ftdi_sio

# confirm ftdi driver is loaded back
ftdi_driver_loaded
if [ $? -eq 0 ]
then
    echo "Failed to load driver !!";
    echo "Exiting...";
    return 1;
fi


COUNTER=0
while [  $COUNTER -lt 10 ]; 
do
    if [ ! -e /dev/tritechUsbSerial0 ]
    then 
        sleep 1  # Waits 1 second
        echo "."; 
    else
        echo "Configuration successfull !!"
        break;
    fi
    COUNTER=$((COUNTER+1))
done

if [  $COUNTER -eq 10 ];
then
    echo "Failed to find Tritech USB serial device !!!"
    echo "Please make sure that Tritech USB serial device has plugged into the computer.."
fi
