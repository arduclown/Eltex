#!/bin/bash

script_name=$(basename "$0")
log_file="report_${script_name}.log"
pipe_path="/tmp/run/cuckoo.pid"

if [[ "$script_name" == "template_task.sh" ]]; then
	echo "я бригадир, сам не работаю"
	exit 0
fi

log() { echo "$(date '+%Y-%m-%d %H:%M:%S') [$$] $1" >> "$log_file" }

if [[ ! -p "$pipe_path" ]]; then
	echo "Канал $pipe_path не найден" >&2
	exit 1
fi

echo "${script_name}[$$]: how much time do I have?" > "$pipe_path" &

read -r sleep_time < "$pipe_path"

sleep "$sleep_time"

log "Скрипт завершился, работал ${sleep_time} секунд."