# IRQSpam

A module that attempts to trigger the regression between 3.2.x and 3.17.

## Compile

    ln -sf <linux_src_tree> linux
    make

## Usage

    insmod irqspam.ko
    # wait a bit
    rmmod irqspam.ko
    dmesg | grep irqspam

## What it does

When loaded, IRQSpam create a kernel thread (irqspam), which is an
infinite loop that triggers a tasklet on the current CPU. The goal
is to trigger as many soft IRQs as possible.

When unloaded, the module prints the number of spawned IRQs.
