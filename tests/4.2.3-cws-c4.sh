#!/bin/sh
#
# Regress structural invariants of the dim-4 combined weight system listing.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -eq 5 ]; then
	SKIP=true
	SKIPREASON="legacy d<=4 auto-generator currently asserts under POLY_Dmax=5"
fi

COMMAND='TMP=$(mktemp) && ./cws-'"${DIM}"'d.x -c4 >"${TMP}" && printf "lines=%s\n" "$(wc -l <"${TMP}")" && printf "first=%s\n" "$(head -n 1 "${TMP}")" && printf "last=%s\n" "$(tail -n 1 "${TMP}")" && rm -f "${TMP}"'
DESCRIPTION="cws-${DIM}d.x -c4 combined CWS invariants"
EXPECTED=$(cat<<-EOF
lines=17320
first=4 1 1 1 1 0 0  4 0 0 1 1 1 1  M:105 6 N:7 6
last=2 1 1 0 0 0 0 0 0  2 0 0 1 1 0 0 0 0  2 0 0 0 0 1 1 0 0  2 0 0 0 0 0 0 1 1  M:81 16 N:9 8
EOF
)
run_test "${SKIP}" "${SKIPREASON}"