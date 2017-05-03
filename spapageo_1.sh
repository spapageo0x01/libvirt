#!/bin/bash

make
sudo make install
sudo service libvirtd start
sudo systemctl daemon-reload
echo "" > /tmp/my_log
