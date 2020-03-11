// Copyright (c) Electroneum Limited 2017-2020
// Copyright 2017 Cedric Mesnil <cslashm@gmail.com>, Ledger SAS

Incomplete Draft of test to perform.



Setup
=====


Note:
-----

Setup has to be done only once, or to restart from fresh config.

Take care to note all your electroneum seeds when generating standard wallet.

`>:` denotes the shell prompt

`[...]: ` denotes the electroneum client prompt


Daemon setup
------------

You can use a local daemons, but it needs you keep them up-to-date and it takes
around 3 days for getting the initial synchronization.

Or you can use a remote daemon. 


**Ledger Daemon setup**


A restricted electroneum daemon runs on ledger server, you can use it with

**Login:**

    >: ETN_LOGIN="--daemon-login <user>:<password>"

Request the LOGIN info over Telegram if you need it.

**Stagenet:**

    >: ETN_DAEMON="--stagenet --daemon-address xx.xxx.xxx.xxx:26967"

**Mainnet:**

    >: ETN_DAEMON="xx.xxx.xxx.xxx:26967"


Client setup
------------

Download client CLI from getelectroneum.org

Unzip the content in a directory of your choice.

Open a Terminal and jump in this directory.

You should have a directory named electroneum-v0.x.y.z, eg: `electroneum-v3.2.0.0`

you can set a variable to that version:

    >: electroneum=./electroneum-v3.2.0.0/electroneum-wallet-cli


Create two wallets
------------------


Launch the ledger electroneum application the device. In `settings->Change Network`, select `stagenet` or `mainnet`
depending the network you are on.

Create a ledger wallet,  in a shell type:

    >:  mkdir -p wallets/device

    >: ${electroneum} \
        ${DAEMON} \
        ${LOGIN} \
        --log-level 4 \
        --restore-height  1 \
        --log-file wallets/device/hwwallet.log  \
        --generate-from-device wallets/device/hwwallet \
        --subaddress-lookahead 2:5


Create a std-wallet, in a shell type:

    >: mkdir -p wallets/std

    >: ${electroneum} \
        ${DAEMON} \
        ${LOGIN} \
        --log-level 4 \
        --restore-height  1 \
        --log-file wallets/std/stdwallet.log  \
        --generate-new-wallet  wallets/std/stdwallet


When asked, accept to Export the view key.

Restoring wallet
----------------

Restoring a device wallet is the same as creating it.

For restoring a std wallet add  `--restore-deterministic-wallet`  option to the creation command.


Create sub address
------------------

At electroneum prompt type:

    [...]: address new
    [...]: address new

(yes twice)

Then:

    [...]: address all

You should get something like that:

    [...]: address all
    0  etnkD4qXHH96PV28NTLdk62V948eoRnSsHGGUXD5aRW8M5mPkPmBFvYWbTFQvqyk2P36wvbahS31z9vZcQgZ8LgP1wHt66nM7A  Primary address (used)
    1  etnk6VQVyxgRxrqf6nCd5xccehTwdaYhrWnZFYPEU8Ju4tsYjFVGfmfSvGS9Sza27gLmrf3i9sS36SqBQ4uJreyY3crDGm1FZC  (Untitled address)
    2  etnk932mEzFJ5nmtwmNUMxd8XqXCzk3sQaMvpMMzrsmFUB2NPkpTmmDaPQzrv7oNv6BaWd2Gv9uAvUAbFD94FQYL4JPmC7Yf8M  (Untitled address)

Entry zero is the main address. The two next are sub-addresses.


Finalize Config and Save All
----------------------------

    [...]: set ask-password 0
    [...]: save
    [...]: exit


Open existing wallet
====================

    >: ${electroneum} \
         ${ETN_DAEMON} \
         ${ETN_LOGIN} \
         --log-level 4 \
         --log-file wallets/std/stdwallet.log  \
         --wallet-file  wallets/std/stdwallet

or

    >: ${electroneum} \
         ${ETN_DAEMON} \
         ${ETN_LOGIN} \
         --log-level 4 \
         --log-file wallets/device/hwwallet.log  \
         --wallet-file  wallets/device/hwwallet


Test Plan
=========


To get initial funds on stagenet, contact us on Telegram.

To get initial funds on mainnet, you will have to buy some Electroneum or earn ETN rewards.


Test 1: transfer
----------------

**Perform**

1. From device wallet to std wallet.

- Make a transfer to only main address
- Make a transfer to only one sub-address
- Make a transfer to two sub-addresses
- Make a transfer to main main address and only one sub-address
- Make a transfer to main main address and two sub-addresses

2. Do the same from std wallet to device wallet


**Check**

For each transfer
 - check the amount is correctly received by the receiver
 - check the change is correctly received by the sender


**Notes**

Transfers are performed with command:

    [...]: transfer <dest_address> amount [ dest_address amount, ...]

Once transfer done you get it status and the change value with:

    [...]: show_transfer <txid>
    or
    [...]: show_transfers


Test 2: sweep to mainaddress
============================

**Perform**

Sweep all from device to std main address

**Check**

- check the amount is correctly received by the receiver


Test 3: sweep to subaddress
===========================

**Perform**

Sweep all from device to std sub-address

**Check**

- check the amount is correctly received by the receiver

