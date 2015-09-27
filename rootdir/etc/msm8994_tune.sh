#!/system/bin/sh +v
#title           :msm8994_tune.sh
#description     :core tunings for Qualcomm MSM8994 platform
#author		 			 :Chang, Ian
#date            :2015/06/05
#version         :0.1
#usage		 		   :sh msm8994_tune.sh
#notes           :Root priviledge is required, it'd be mandatory to 
#									turn off Selinux enforce
#=================================================
# ensure at least one little core is online
lcores=( 0 1 2 3 -1 )

# tune governors for little cores
for lcore in ${lcores[@]}; do
	[ "$lcore" == "-1" ] && break
	
	if [ -e "/sys/devices/system/cpu/cpu${lcore}/cpufreq" ]; then
		echo "19000 960000:39000 1248000:29000" > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/above_hispeed_delay
		echo 95 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/go_hispeed_load
                echo 960000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/hispeed_freq
		echo "80 960000:95 1248000:99" > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/target_loads
		echo 1 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/use_sched_load
		break
	fi
done

# ensure at least one big core is online
bcores=( 4 5 6 7 -1 )

# tune governors for big cores
for bcore in ${bcores[@]}; do
	[ "$bcore" == "-1" ] && break

	if [ -w "/sys/devices/system/cpu/cpu${bcore}/cpufreq/interactive/target_loads" ]; then
		echo "90 1248000:95 1800000:70" > /sys/devices/system/cpu/cpu${bcore}/cpufreq/interactive/target_loads
		break
	fi
done

# enable H-Cube tunings for big cores
echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/hc_on
echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/hc_down_scale_on
echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/hc_thres_on
echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/hc_ro_use_gourd
echo "0 0 768000 1248000" > /sys/devices/system/cpu/cpu4/core_ctl/hc_correl_lvs
echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/hc_correl_use_pcost