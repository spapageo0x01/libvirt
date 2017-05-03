#!/bin/bash

make
sudo make install
sudo service libvirtd stop
sudo service libvirtd start
cp iof_glue.sh /usr/iof
#sudo systemctl daemon-reload
echo "" > /tmp/my_log
