#!/bin/sh
#
# Regress the dim-5 two-file overlap-3 constructor path.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
    SKIP=true
    SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND="./cws-${DIM}d.x -c5 -n2 tests/input/4.2.7-cws-c5-overlap3.txt tests/input/4.2.7-cws-c5-overlap3.txt -t 3 3"
DESCRIPTION="cws-${DIM}d.x -c5 -n2 overlap-3 two-file enumeration"
EXPECTED=$(cat<<-EOF
7 2 2 1 1 1 0 0  7 0 0 1 1 1 2 2  M:258 15 N:9 8
7 2 2 1 1 1 0 0  7 0 0 1 1 2 1 2  M:163 24 N:12 10
7 2 2 1 1 1 0 0  7 0 0 1 2 2 1 1  M:187 25 N:12 9
7 2 1 2 1 1 0 0  7 0 0 1 1 2 1 2  M:209 30 F:12 N:10
7 2 1 2 1 1 0 0  7 0 0 1 2 1 1 2  M:209 30 F:12 N:10
7 2 1 2 1 1 0 0  7 0 0 1 2 2 1 1  M:262 26 N:12 10
7 2 1 2 1 1 0 0  7 0 0 2 2 1 1 1  M:311 22 F:10 N:10
7 1 1 2 2 1 0 0  7 0 0 1 2 2 1 1  M:413 21 N:11 10
7 1 1 2 2 1 0 0  7 0 0 2 1 2 1 1  M:413 21 N:11 10
EOF
)
run_test "${SKIP}" "${SKIPREASON}"