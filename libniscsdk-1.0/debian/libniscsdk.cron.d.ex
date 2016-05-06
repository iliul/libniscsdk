#
# Regular cron jobs for the libniscsdk package
#
0 4	* * *	root	[ -x /usr/bin/libniscsdk_maintenance ] && /usr/bin/libniscsdk_maintenance
