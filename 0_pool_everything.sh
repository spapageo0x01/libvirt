#!/bin/bash

#virsh pool-define-as death1 vicinity --source-host 172.16.0.31:1340 --auth-username spapageo --secret-usage 112358
virsh pool-define-as death1 vicinity --source-host 172.16.0.31 --auth-username spapageo

virsh pool-start death1

virsh pool-list --all
