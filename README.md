<p align="center">
  <img src="https://imgur.com/3FpIaYL.png">
</p>
<h4 align="center">Electroneum is a Fast, Secure, Mobile Based Cryptocurrency </h4>

## Table of Contents

* [Introduction](#Introduction)
* [About This Project](#About-This-Project)
  * [What is a Hardware Wallet?](#What-is-a-Hardware-Wallet)
* [Development Resources](#Development-Resources)
  * [Setting up the Toolchain](#Setting-up-the-Toolchain)
  * [Setting up the SDK](#Setting-up-the-SDK)
  * [Python Loader](#Python-Loader)
  * [Building and Loading Apps](#Building-and-Loading-Apps)
  * [Install from Source](#Install-from-Source)
* [Revision](#Revision)
* [License](#License)
* [Copyright](#Copyright)

## Introduction

Electroneum uses a cryptographically sound system to allow you to send and receive your tokens without your transactions being easily revealed on the blockchain. This ensures that all token transfers remain absolutely private by default, but if necessary, can be proven to a third party by providing specific keys.

**Security:** Using the power of a distributed peer-to-peer consensus network, every transaction on the network is cryptographically secured. Individual wallets have a 25 word mnemonic seed that is only displayed once, and can be written down to backup the wallet. Wallet files are encrypted with a passphrase to ensure they are useless if stolen.

Electroneum (and its group companies) have separately developed proprietary software which can be used in conjunction with this project. This software is subject to separate terms and conditions which are available at https://electroneum.com .

# About this Project

This is the Ledger app implementation for Electroneum wallets. It is open source and completely free to use without restrictions, except for those specified in the license agreement below.

## What is a Hardware Wallet?

A hardware wallet is a cryptocurrency wallet which stores the user's private keys (critical piece of information used to authorise outgoing transactions on the blockchain network) in a secure hardware device. The main principle behind hardware wallets is to provide full isolation between the private keys and your easy-to-hack computer or smartphone.



<p align="center">
    <img width="650" height="90" src="https://imgur.com/bF7IQM7.png">
</p>

* **The first & only certified hardware wallet on the market**
Ledger is the first and only certified hardware wallet on the market, certified for its security by ANSSI, the French cyber security agency.

* **Integrates a Secure Element (SE), the most secured chip**
Ledger hardware wallets integrate a certified chip, designed to withstand sophisticated attacks, and capable of securely hosting cryptographic data such as private keys.

* **The only device with a custom Operating System for more protection**
Ledger wallets are the only hardware wallet to have their own custom OS (BOLOS) to protect the device against malicious attacks and isolate applications from each other.

* **Genuine check to assure your device integrity at all time**
The genuine check developed by Ledger is an authentication ensuring that your Ledger device has not been tampered with or compromised by a third party.

**See more at [Why should you choose Ledger hardware wallets](https://www.ledger.com/academy/hardwarewallet/why-you-should-choose-ledger-hardware-wallets/)**

# Development Resources

**Only Linux is supported as a development OS. For Windows and MacOS users, a Linux VM is recommended.**

Developing and/or compiling BOLOS applications requires the SDK matching the appropriate device (the Nano S SDK or the Blue SDK) as well as the following two compilers:

* A standard ARM gcc to build the non-secure (STM32) firmware and link the secure (ST31) applications
* A standard ARM clang above 7.0.0 with [ROPI support](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0491i/CHDCDGGG.html) to build the secure (ST31) applications
* Download a prebuilt gcc from [here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

## Setting up the Toolchain

The Makefiles used by our BOLOS applications look for the gcc and clang installations using the following process:

1. If the ```BOLOS_ENV``` environment variable is set, then gcc is used from ```$BOLOS_ENV/gcc-arm-none-eabi-5_3-2016q1/bin/``` and clang is used from ```$BOLOS_ENV/clang-arm-fropi/bin/```.

1. As a fallback, if ```BOLOS_ENV``` is not set, then gcc is used from ```GCCPATH``` and clang is used from ```CLANGPATH```.

1. As a fallback, if either ```GCCPATH``` or ```CLANGPATH``` is not set, then gcc and clang, respectively, are used from the PATH.

This allows you to setup both gcc and clang under the same directory and reference it using ```BOLOS_ENV```, or configure where each compiler is looked for individually. If your system already has an appropriate version of clang installed, you may simply leave ```BOLOS_ENV``` and ```CLANGPATH``` unset and clang will be used from the PATH (but make sure to set ```GCCPATH```).

If you’re just looking for a one-size-fits-all solution to satisfy your toolchain needs, here are the steps you should follow:

1. Choose a directory for the BOLOS environment (I’ll use ```~/bolos-devenv/```) and link the environment variable ```BOLOS_ENV``` to this directory.

1. Download a prebuilt gcc from https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads and unpack it into ```~/bolos-devenv/```. Make sure there is a directory named ```bin``` directly inside ```~/bolos-devenv/gcc-arm-none-eabi-5_3-2016q1/```.
1. Download a prebuilt clang from http://releases.llvm.org/download.html#7.0.0 and unpack it into ```~/bolos-devenv/```. Rename the directory that was inside the archive you downloaded to ```clang-arm-fropi```, or create a link to the directory with that name. Make sure there is a directory named bin directly inside ```~/bolos-devenv/clang-arm-fropi/```.

**Note:** Not all of the Makefiles for our applications available on GitHub may recognize ```BOLOS_ENV``` in the way described above. If the Makefile is having trouble finding the right compilers, try setting ```GCCPATH``` and ```CLANGPATH``` explicitly.

Cross compilation headers are required and provided within the gcc-multilib and g++-multilib packages. To install them on a debian system:

```sudo apt install gcc-multilib g++-multilib```

If your OS doesn't come bundled with the pillow package, you may receive errors whilst building. In this case, you should be able to remedy the situation with

```pip3 install pillow```

## Setting up the SDK

Now that you have your toolchain set up, you need to download / clone the SDK for the appropriate Ledger device you’re working with. You can do this anywhere, it doesn’t have to be in your ```BOLOS_ENV``` directory (if you even have one). Make sure you checkout the tag matching your firmware version.

Ledger Nano S SDK: https://github.com/LedgerHQ/nanos-secure-sdk

Ledger Blue SDK: https://github.com/LedgerHQ/blue-secure-sdk

Finally, link the environment variable ```BOLOS_SDK``` to the SDK you downloaded. When using the Makefile for our BOLOS apps, the Makefile will use the contents of the SDK to determine your target device ID (Ledger Nano S or Ledger Blue). Even if you aren’t building an app, loading an app with the Makefile still requires you to have the SDK for the appropriate device linked to by ```BOLOS_SDK```.

## Python Loader
If you intend to communicate with an actual Ledger device from a host computer at all,you will need the Python loader installed. For more information on installing and using the Python loader, see [BOLOS Python Loader](https://ledger.readthedocs.io/projects/blue-loader-python/en/0.1.15/index.html). The Makefiles for most of our apps interface with the Python loader directly, so if you only need to load / delete apps then you don’t need to know how to use the various scripts provided by the Python loader, but you’ll still need it installed.

## Building and Loading Apps

In this section, we’ll walk you through compiling and loading the Electroneum BOLOS app onto your device. Applications that support multiple BOLOS devices are typically contained within a single repository, so you can use the same repository to build an app for different Ledger devices. Just make sure that you’ve set ```BOLOS_SDK``` to the appropriate SDK for the device you’re using. The Makefiles used by our apps use the contents of the SDK to determine which device you’re using.

Firstly, download the electroneum app.

```git clone https://github.com/electroneum/electroneum-ledger-app.git```

Now you can let the Makefile do all the work. The load target will build the app if necessary and load it onto your device over USB.

```
cd electroneum-ledger-app/
make load
```

And you’re done! After confirming the installation on your device, you should see an app named Electroneum. The app can be deleted like so:

```make delete```

## Install from Source

In order to install from source for testing purpose you need to uncomment the two following lines in Makefile

    DEFINES   += DEBUG_HWDEVICE
    DEFINES   += IODUMMYCRYPT

Note this is only for testing. For production usage, use the application provided by the Live Manager.

# Revision

## V1.0.0

Targeted Client: Electroneum v3.2.0.0

Targeted Firmware: SE 1.6.0 / MCU 1.11

* Initial Release. 

# License

See [LICENSE](LICENSE).

# Copyright

Copyright (c) 2017-2020, The Electroneum Project

Copyright (c) 2017-2019, CslashM (Ledger)
