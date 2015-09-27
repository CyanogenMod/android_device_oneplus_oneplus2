#!/system/bin/sh +v
#title           :msm8994_hmp.sh
#description     :core tunings for Qualcomm MSM8994 platform OnePlus 2
#author          :Gascoigne, Aaron | Nguyen, Hieu
#date            :2015/08/06
#version         :1.1
#usage           :sh msm8994_hmp.sh
#=================================================
# ensure at least one little core is online
lcores=( 0 1 2 3 -1 )

# tune governors for little cores
for lcore in ${lcores[@]}; do
        [ "$lcore" == "-1" ] && break

        if [ -e "/sys/devices/system/cpu/cpu${lcore}/cpufreq" ]; then
        echo 5 > /sys/devices/system/cpu/cpu${lcore}/sched_mostly_idle_nr_run
        echo 60 > /sys/devices/system/cpu/cpu${lcore}/sched_mostly_idle_load
        echo 960000 > /sys/devices/system/cpu/cpu${lcore}/sched_mostly_idle_freq
        echo 0 > /sys/devices/system/cpu/cpu${lcore}/sched_prefer_idle

        echo 10000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/timer_rate
        echo 20000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/min_sample_time
        echo 20000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/max_freq_hysteresis
        break
        fi
done

# ensure at least one big core is online
bcores=( 4 5 6 7 -1 )

# tune governors for big cores
for bcore in ${bcores[@]}; do
        [ "$bcore" == "-1" ] && break

        if [ -w "/sys/devices/system/cpu/cpu$/cpu${bcore}/cpufreq/interactive/target_loads" ]; then
        echo 3 > /sys/devices/system/cpu/cpu${bcore}/sched_mostly_idle_nr_run
        echo 20 > /sys/devices/system/cpu/cpu${bcore}/sched_mostly_idle_load
        echo 0 > /sys/devices/system/cpu/cpu${bcore}/sched_mostly_idle_freq
        echo 0 > /sys/devices/system/cpu/cpu${bcore}/sched_prefer_idle

        echo 10000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/timer_rate
        echo 600000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/hispeed_freq
        echo 10000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/min_sample_time
        echo 20000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/max_freq_hysteresis
        echo 10000 > /sys/devices/system/cpu/cpu${lcore}/cpufreq/interactive/timer_slack
        break
        fi
done

# HMP Background migration
echo 9 > /proc/sys/kernel/sched_upmigrate_min_nice
echo 90 > /proc/sys/kernel/sched_downmigrate
echo 60 > /proc/sys/kernel/sched_small_task
echo 30 > /proc/sys/kernel/sched_init_task_load
