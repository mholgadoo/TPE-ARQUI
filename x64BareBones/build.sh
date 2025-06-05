#!/bin/bash

docker start tpe-container
docker exec -it tpe-container make clean -C /root/Toolchain
docker exec -it tpe-container make clean -C /root/
docker exec -it tpe-container make -C /root/Toolchain
docker exec -it tpe-container make -C /root/
docker stop tpe-container
