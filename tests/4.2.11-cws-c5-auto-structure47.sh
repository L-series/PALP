#!/bin/sh
#
# Regress the automatic dim-5 dispatcher on the smallest builtin five-input
# structure.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND="./cws-${DIM}d.x -c5 -s47"
DESCRIPTION="cws-${DIM}d.x -c5 automatic structure 47"
EXPECTED=$(cat<<-EOF
2 1 1 0 0 0 0 0 0 0 0  2 0 0 1 1 0 0 0 0 0 0  2 0 0 0 0 1 1 0 0 0 0  2 0 0 0 0 0 0 1 1 0 0  2 0 0 0 0 0 0 0 0 1 1  M:243 32 N:11 10
EOF
)
run_test "${SKIP}" "${SKIPREASON}"