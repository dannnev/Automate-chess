# Introduction
Automate chess is a variant of chess game where players set up the starting position themselves and then watch the computer playing the best moves. It was first presented to players on chess.com, however later was removed from the website. This project is a recreation of automate chess, made for playing on a single device.

# Requirements
To launch the game you will first need to download stockfish engine on your device. The code communicates with stockfish to receive the best move through the game.
[Instructions for downloading stockfish](https://linux.how2shout.com/how-to-install-stockfish-on-ubuntu-22-04-or-20-04-a-chess-engine/)

Then you have to make files which communicate with stockfish executable. 
```bash
chmod +x hardthink.sh
chmod +x middlethink.sh
chmod +x easythink.sh
chmod +x sponge.sh
chmod +x pusk.sh
```

# Important
After installing stockfish please examine how much unused RAM your device currently posses. During the game the files hardthink.sh, middlethink.sh, easythink.sh will send a command to stockfish allowing to use 8192Mb of RAM. If you do not have that much RAM available for stockfish to use you can change it yourself. Otherwise be aware that your device may freeze.

```bash

set list [exec cat fen.txt]
spawn stockfish
expect -timeout 1  readyok
send "setoption name Hash value 8192 \r"
expect -timeout 1 readyok
send "position fen $list \r"
expect -timeout 1  readyok
send "go movetime 10000 \r"
log_file script.txt
expect -timeout 12 readyok

```

# Launch

To start the game go to folder "chess" and run "pusk.sh" as a program. Here is a video of the game works. [Automate chess](https://youtu.be/rXoDWlpO1rw)

# License

[MIT](https://choosealicense.com/licenses/mit/)