#!/bin/sh
#
# Regress the complete dim-3 combined weight system listing.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -eq 5 ]; then
	SKIP=true
	SKIPREASON="legacy d<=4 auto-generator currently asserts under POLY_Dmax=5"
fi

COMMAND="./cws-${DIM}d.x -c3"
DESCRIPTION="cws-${DIM}d.x -c3 combined CWS enumeration"
EXPECTED=$(cat<<-EOF
3 1 1 1 0 0  3 0 0 1 1 1  M:30 5 N:6 5
3 1 1 1 0 0  4 0 0 1 1 2  M:23 7 N:8 6
3 1 1 1 0 0  4 0 0 2 1 1  M:31 6 N:7 5
3 1 1 1 0 0  6 0 0 1 2 3  M:14 7 N:11 6
3 1 1 1 0 0  6 0 0 2 1 3  M:21 5 N:9 5
3 1 1 1 0 0  6 0 0 3 1 2  M:24 6 N:9 5
4 2 1 1 0 0  4 0 0 1 1 2  M:19 5 N:9 5
4 2 1 1 0 0  4 0 0 2 1 1  M:23 6 N:9 5
4 2 1 1 0 0  6 0 0 1 2 3  M:12 6 N:14 6
4 2 1 1 0 0  6 0 0 2 1 3  M:16 6 N:14 6
4 2 1 1 0 0  6 0 0 3 1 2  M:18 6 N:12 5
4 1 1 2 0 0  4 0 0 2 1 1  M:35 5 N:7 5
4 1 1 2 0 0  6 0 0 3 1 2  M:27 5 N:9 5
6 3 2 1 0 0  6 0 0 1 2 3  M:9 5 N:18 5
6 3 2 1 0 0  6 0 0 2 1 3  M:10 6 N:20 6
6 3 1 2 0 0  6 0 0 2 1 3  M:15 5 N:15 5
6 2 1 3 0 0  6 0 0 3 1 2  M:21 5 N:12 5
2 1 1 0 0 0  3 0 0 1 1 1  M:30 6 N:6 5
2 1 1 0 0 0  4 0 0 1 1 2  M:27 6 N:7 5
2 1 1 0 0 0  6 0 0 1 2 3  M:21 6 N:9 5
2 1 1 0 0 0 0  2 0 0 1 1 0 0  2 0 0 0 0 1 1  M:27 8 N:7 6
EOF
)
run_test "${SKIP}" "${SKIPREASON}"