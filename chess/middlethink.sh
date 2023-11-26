#!/bin/expect

set list [exec cat fen.txt]
spawn stockfish
expect -timeout 1  readyok
send "setoption name Hash value 8192 \r"
expect -timeout 1 readyok
send "position fen $list \r"
expect -timeout 1  readyok
send "go movetime 4000 \r"
log_file script.txt
expect -timeout 6 readyok