#!/system/bin/sh
#
# start ril-daemon only for targets on which radio is present
#

multisim=`getprop persist.radio.multisim.config`

if [ "$multisim" = "dsds" ] || [ "$multisim" = "dsda" ]; then
    start ril-daemon2
fi
