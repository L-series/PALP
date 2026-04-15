#!/bin/sh
#
# Regress structural invariants of the automatic dim-5 dispatcher on a
# builtin structure that exercises the builtin 3- and 4-weight sources.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND='TMP=$(mktemp) && ./cws-'${DIM}'d.x -c5 -s5 >"${TMP}" && printf "lines=%s\n" "$(wc -l <"${TMP}")" && printf "first=%s\n" "$(head -n 1 "${TMP}")" && printf "last=%s\n" "$(tail -n 1 "${TMP}")" && rm -f "${TMP}"'
DESCRIPTION="cws-${DIM}d.x -c5 automatic structure 5 invariants"
EXPECTED=$(cat<<-EOF
lines=285
first=3 1 1 1 0 0 0 0  4 0 0 0 1 1 1 1  M:350 12 N:8 7
last=6 1 2 3 0 0 0 0  66 0 0 0 5 6 22 33  M:63 12 N:45 7
EOF
)
run_test "${SKIP}" "${SKIPREASON}"