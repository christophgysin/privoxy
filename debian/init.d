#! /bin/sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/sbin/privoxy
NAME=privoxy
DESC="filtering proxy server"
OWNER=privoxy
CONFIGFILE=/etc/privoxy/config
PIDFILE=/var/run/$NAME.pid

test -f $DAEMON || exit 0

set -e

case "$1" in
  start)
	echo -n "Starting $DESC: "
	start-stop-daemon --oknodo --start --quiet --pidfile $PIDFILE \
	    --exec $DAEMON -- --pidfile $PIDFILE --user $OWNER $CONFIGFILE \
	    2>> /var/log/privoxy/errorfile
 	echo "$NAME."
	;;
  stop)
	echo -n "Stopping $DESC: "
	start-stop-daemon --oknodo --stop --quiet --pidfile $PIDFILE \
		--exec $DAEMON
	rm -f $PIDFILE
	echo "$NAME."
	;;
  #reload)
	#
	#	If the daemon can reload its config files on the fly
	#	for example by sending it SIGHUP, do it here.
	#
	#	If the daemon responds to changes in its config file
	#	directly anyway, make this a do-nothing entry.
	#
	# echo "Reloading $DESC configuration files."
	# start-stop-daemon --stop --signal 1 --quiet --pidfile \
	#	/var/run/$NAME.pid --exec $DAEMON
  #;;
  restart|force-reload)
	#
	#	If the "reload" option is implemented, move the "force-reload"
	#	option to the "reload" entry above. If not, "force-reload" is
	#	just the same as "restart".
	#
	echo -n "Restarting $DESC: "
	start-stop-daemon --oknodo --stop --quiet --pidfile $PIDFILE \
		--exec $DAEMON
	sleep 1
	start-stop-daemon --oknodo --start --quiet --pidfile $PIDFILE \
	    --exec $DAEMON -- --pidfile $PIDFILE --user $OWNER $CONFIGFILE \
	    2>> /var/log/privoxy/errorfile
	echo "$NAME."
	;;
  *)
	N=/etc/init.d/$NAME
	# echo "Usage: $N {start|stop|restart|reload|force-reload}" >&2
	echo "Usage: $N {start|stop|restart|force-reload}" >&2
	exit 1
	;;
esac

exit 0
