# aCoral-I RISC-V command line version

[![Build Status](https://travis-ci.org/kendryte/kendryte-standalone-sdk.svg)](https://travis-ci.org/kendryte/kendryte-standalone-sdk)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

This is aCoral-I single-core version on k210 board,developed with Kendryte K210 standalone SDK. Attention that the nncase runtime version here is "v0" which you may find in the newest repo of "K210 standalone SDK" along with "v1". 

---
## I. TOC
```
--build  //cmake build target dir
  |--acoral  //elf file
  |--acoral.bin  //bin file
--cmake  //cmake scripts
--lds
  |--kendryte.ld  //link script
--lib
  |--bsp  //include boot file crt.S
  |--drivers  //k210 hardware drivers
  |--nncase  //neural network runtime lib on k210's KPU
--src
  |--acoral  //aCoral-I source code
--CMakeList.txt  //cmake command entrance
--sipeed-rv-debugger.cfg  //openocd configuration
```

---
## II. Prerequisite
### 1. install toolchain
download [here](https://github.com/kendryte/kendryte-gnu-toolchain/releases/tag/v8.2.0-20190213), add 'bin' directory to System Enviroment Path, e.g.
``` 
C:\Users\SPGGO\Documents\aCoral\kendryte-toolchain\bin
```
PS : remember to copy and rename the file "\bin\mingw32-make" to "make", so that vscode plugin could recognize it.

<br/>

### 2. install openocd
download [here](https://github.com/kendryte/openocd-kendryte/releases/tag/v0.2.3), add 'bin' directory to System Enviroment Path, e.g.
``` 
C:\Users\SPGGO\Documents\aCoral\tool-openocd\bin
```
<br/>

### 3. install cmake
download [here](https://cmake.org/download/), and don't forget to add it to System Path.

<br/>

### 4. install VScode plugin "CMake Tools"
easy to configure and generate program in VScode.

<br/>

---
## III. Usage

If you want to make change of cmake and create a new Makefile, delete 'build' directory, or you even don't have a 'build' dir,  run commands below in root directory of project.

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DPROJ=acoral
make
```

You will get 2 key files in build dir, `acoral` and `acoral.bin`.

* If you are using want to debug your program, use `acoral`. You need to replace two strings after "-file-exec-and-symbols" in launch.json
  ```
  C:/Users/SPGGO/Documents/aCoral/aCoral-riscv-cl/build/acoral
  ``` 
  to
  ```
  /your/acoral/elf/loaction
  ```
<br/>

* If you want to flash it onto k210, use `acoral.bin`. Flash tool could be download download [here](https://github.com/sipeed/kflash_gui/releases). Remember to press 'BOOT' button on board while pressing 'RESET', that is for entering ISP Download Mode.Baud rate need to be set as 115200.

---
## IV. aCoral-I User Programming API
You can find aCoral programming API [here](API.md)

---
e-mail: SPGGOGOGO@outlook.com
