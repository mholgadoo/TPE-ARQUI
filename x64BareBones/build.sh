#!/bin/bash

docker start tpe-arqui
docker exec -it tpe-arqui make clean -C /root/x64BareBones/Toolchain
docker exec -it tpe-arqui make clean -C /root/x64BareBones
docker exec -it tpe-arqui make -C /root/x64BareBones/Toolchain
docker exec -it tpe-arqui make -C /root/x64BareBones
docker stop tpe-arqui
