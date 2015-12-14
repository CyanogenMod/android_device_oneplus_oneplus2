#!/system/bin/sh
#
# start ril-daemon only for targets on which radio is present
#

start qmuxd
start ipacm-diag
start ipacm

datamode=`getprop persist.data.mode`
netmgr=`getprop ro.use_data_netmgrd`
multisim=`getprop persist.radio.multisim.config`

if [ "$multisim" = "dsds" ] || [ "$multisim" = "dsda" ]; then
    start ril-daemon2
fi

case "$datamode" in
    "tethered")
        start qti
        start port-bridge
        ;;
    "concurrent")
        start qti
        if [ "$netmgr" = "true" ]; then
            start netmgrd
        fi
        ;;
    *)
        if [ "$netmgr" = "true" ]; then
            start netmgrd
        fi
        ;;
esac
