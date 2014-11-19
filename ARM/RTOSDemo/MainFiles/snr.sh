#!/bin/bash

grep -rl '$2' | xargs sed -i 's/$1/$2/'
