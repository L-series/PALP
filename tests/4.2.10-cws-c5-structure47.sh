#!/bin/sh
#
# Regress the generic dim-5 five-input structure selector on the fully
# disjoint five-segment case.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND="./cws-${DIM}d.x -c5 -n5 tests/input/4.2.8-cws-w2.txt tests/input/4.2.8-cws-w2.txt tests/input/4.2.8-cws-w2.txt tests/input/4.2.8-cws-w2.txt tests/input/4.2.8-cws-w2.txt -s47"
DESCRIPTION="cws-${DIM}d.x -c5 generic five-input structure 47"
EXPECTED=$(cat<<-EOF
2 1 1 0 0 0 0 0 0 0 0  2 0 0 1 1 0 0 0 0 0 0  2 0 0 0 0 1 1 0 0 0 0  2 0 0 0 0 0 0 1 1 0 0  2 0 0 0 0 0 0 0 0 1 1  M:243 32 N:11 10
EOF
)
run_test "${SKIP}" "${SKIPREASON}"