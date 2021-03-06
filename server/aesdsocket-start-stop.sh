#! /bin/sh

# Reference: https://man7.org/linux/man-pages/man8/start-stop-daemon.8.html (Example)

case "$1" in
  start)
    echo "Starting aesdsocket"
    start-stop-daemon -S -n aesdsocket -x /usr/bin/aesdsocket -- -d
    ;;
  stop)
    echo "Stopping aesdsocket"
    start-stop-daemon -K -n aesdsocket 
    ;;
  *)
    echo "Usage: $0 {start|stop}"
  exit 1
esac

exit 0
