#!/bin/bash

set -x 
cd "$(dirname "${BASH_SOURCE[0]}")"
dd if=outimg.img of=/dev/sdd1 

