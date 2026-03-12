#!/bin/bash

docker run -it "anmaped/rmtld3synth"


rmtld3synth --synth-cpp11 --input-dsl "duration of a in 0 .. 15 >= 5" --out-src out


docker ps


docker cp 82b195cbd99c:/root/out /home/bruno/Desktop/Projeto/monito
r/programa/output