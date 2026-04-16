#!/bin/sh
#
# Regress the generic dim-5 three-input structure selector on a structure
# involving a 5-weight input.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND="./cws-${DIM}d.x -c5 -n3 tests/input/4.2.8-cws-w2.txt tests/input/4.2.7-cws-c5-overlap3.txt tests/input/4.2.8-cws-w4.txt -s15"
DESCRIPTION="cws-${DIM}d.x -c5 generic three-input structure 15"
EXPECTED=$(cat<<-EOF
2 1 1 0 0 0 0 0 0  7 1 2 1 1 2 0 0 0  4 0 0 1 0 0 1 1 1  M:234 29 F:11 N:11
2 1 1 0 0 0 0 0 0  7 1 2 2 1 1 0 0 0  4 0 0 1 0 0 1 1 1  M:333 27 N:11 10
2 1 1 0 0 0 0 0 0  7 2 2 1 1 1 0 0 0  4 0 0 1 0 0 1 1 1  M:315 18 N:9 8
EOF
)
run_test "${SKIP}" "${SKIPREASON}"