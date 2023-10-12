#!/bin/bash

# usage: addenv env_name path
function addenv() {
  sed -i -e "/^export $1=.*/d" ~/.bashrc
  echo "export $1=`readlink -e $2`" >> ~/.bashrc
  echo "By default this script will add environment variables into ~/.bashrc."
  echo "After that, please run 'source ~/.bashrc' to let these variables take effect."
  echo "If you use shell other than bash, please add these environment variables manually."
}

# usage: init repo branch directory trace [env]
# trace = true|false
function init() {
  if [ $5 ] ; then
    addenv $5 $3
  fi
}

init NJU-ProjectN/nemu ics2023 nemu true NEMU_HOME
init NJU-ProjectN/abstract-machine ics2023 abstract-machine true AM_HOME
init NJU-ProjectN/fceux-am ics2021 fceux-am false
init NJU-ProjectN/am-kernels ics2021 am-kernels false
# init NJU-ProjectN/nanos-lite ics2021 nanos-lite true
# init NJU-ProjectN/navy-apps ics2023 navy-apps true NAVY_HOME