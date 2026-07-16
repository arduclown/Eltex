#!/bin/bash

config_file="observer.conf"
log_file="observer.log"

while read -r script_path; do
    running=0

    for cmdline in /proc/[0-9]*/cmdline; do
        if tr '\0' ' ' < "$cmdline" 2>/dev/null |
           grep -Fq "$script_path"; then
            running=1
            break
        fi
    done

    if [ "$running" -eq 0 ]; then
        nohup bash "$script_path" >/dev/null 2>&1 &
        echo "$(date) Перезапущен $script_path" >> "$log_file"
    fi
done < "$config_file"