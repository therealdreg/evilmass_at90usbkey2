#!/bin/bash

set -x 
cd "$(dirname "${BASH_SOURCE[0]}")"
dd if=/dev/sdd1 of=crealdump.img


