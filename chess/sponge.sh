#!/bin/bash

timeout 0.1s tail -n 1 script.txt | sponge script.txt
head -c 14 script.txt | sponge script.txt
tail -c 5 script.txt | sponge script.txt