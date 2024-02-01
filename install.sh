#!/bin/bash

apt-get update
apt-get install -y wget 
apt-get install -y curl 
apt-get install -y vim
apt-get install -y tmux
apt-get install -y git
apt-get install -y sudo

# NJU PA
apt-get install -y build-essential
apt-get install -y gdb
apt-get install -y libreadline-dev
apt-get install -y libsdl2-dev
apt-get install -y llvm llvm-dev
apt-get install -y bison flex

apt-get install -y openssh-server

apt-get install -y device-tree-compiler

apt-get install -y scons

echo "Install is complete."
