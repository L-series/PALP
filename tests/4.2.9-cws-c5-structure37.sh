#!/bin/sh
#
# Regress the generic dim-5 four-input structure selector on a small
# disjoint structure.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND="./cws-${DIM}d.x -c5 -n4 tests/input/4.2.8-cws-w2.txt tests/input/4.2.8-cws-w2.txt tests/input/4.2.8-cws-w2.txt tests/input/4.2.8-cws-w3.txt -s37"
DESCRIPTION="cws-${DIM}d.x -c5 generic four-input structure 37"
EXPECTED=$(cat<<-EOF
2 1 1 0 0 0 0 0 0 0  2 0 0 1 1 0 0 0 0 0  2 0 0 0 0 1 1 0 0 0  3 0 0 0 0 0 0 1 1 1  M:270 24 N:10 9
EOF
)
run_test "${SKIP}" "${SKIPREASON}"