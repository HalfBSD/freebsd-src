#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Host-side test harness: writes a private overlay, never the prepared base disk.
set -eu
base=$(realpath "${1:?usage: vm-cycles.sh prepared-base-image new-results-directory [cycles]}")
results=${2:?new results directory required}
cycles=${3:-25}
case "$cycles" in ''|*[!0-9]*|0) echo 'Positive cycle count required' >&2; exit 1;; esac
format=${HALFBSD_VM_FORMAT:-qcow2}
case "$format" in raw|qcow2) ;; *) echo 'Supported formats: raw, qcow2' >&2; exit 1;; esac
command -v qemu-system-x86_64 >/dev/null
command -v qemu-img >/dev/null
command -v timeout >/dev/null
command -v rg >/dev/null
[ -f "$base" ] && [ ! -e "$results" ]
mkdir -m 700 "$results"
results=$(realpath "$results")
qemu-img create -f qcow2 -F "$format" -b "$base" "$results/overlay.qcow2"
printf 'cycle\tresult\tready_record\n' > "$results/cycles.tsv"
i=1
while [ "$i" -le "$cycles" ]; do
    log="$results/cycle-$i.log"
    set -- qemu-system-x86_64 -m 4096 -smp 2 -display none -monitor none \
        -serial stdio -no-reboot -drive "file=$results/overlay.qcow2,format=qcow2,if=virtio" \
        -nic user,model=virtio-net-pci
    if [ -n "${HALFBSD_VM_BIOS:-}" ]; then set -- "$@" -bios "$HALFBSD_VM_BIOS"; fi
    if timeout -k 10s "${HALFBSD_VM_TIMEOUT:-900}" "$@" > "$log" 2>&1; then
        if rg -q 'HALFBSD_PHASE4_VM_READY monotonic=' "$log" && \
            rg -q '"cleaned":true,"event":"boot.shutdown"|"event":"boot.shutdown","cleaned":true' "$log"; then
            record=$(sed -n '/HALFBSD_PHASE4_VM_READY monotonic=/p' "$log" | head -1)
            printf '%s\tPASS\t%s\n' "$i" "$record" >> "$results/cycles.tsv"
        else printf '%s\tFAIL-readiness-or-cleanup\t\n' "$i" >> "$results/cycles.tsv"; exit 1; fi
    else printf '%s\tFAIL-vm-exit-or-timeout\t\n' "$i" >> "$results/cycles.tsv"; exit 1; fi
    i=$((i+1))
done
printf 'PASS %s persistent-overlay boot/reboot cycles; inspect logs and run failure matrix separately\n' "$cycles"
