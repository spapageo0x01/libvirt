#!/bin/bash

virsh pool-define-as death1 vicinity

virsh pool-build death1

virsh pool-start death1

virsh pool-list --all
