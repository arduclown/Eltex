#!/bin/bash

log_file="cuckoo.log"
pipe_path="/tmp/run/cuckoo.pid"
mkdir -p /tmp/run
if [[ ! -p "$pipe_path" ]]; then
       	if [[ -e "$pipe_path" ]]; then 
       		rm -f "$pipe_path"
 	fi

	mkfifo "$pipe_path"
fi

log() {
	echo "$(date '+%Y-%m-%d %H:%M:%S') $1" >> "$log_file"
}

trap 'log "Shutdown!"; rm -f /tmp/run/cuckoo.pid; exit' SIGTERM SIGINT

log "Startup!"
echo "Cuckoo запущен. PID: $$"

while true; do
	read -r msg < "$pipe_path"
	if [[ "$msg" =~ ^([^[]+)\[([0-9]+)\]:\ how\ much\ time\ do\ I\ have\?$ ]]; then
        name="${BASH_REMATCH[1]}"
        pid="${BASH_REMATCH[2]}"
        n=$((RANDOM % 9 + 2))
        echo "$n" > "$pipe_path" 
        log "${name}[${pid}] $n" 
    fi
done