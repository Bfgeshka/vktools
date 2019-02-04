# vkgrab
Grab everything you can from vk.com pages! Works with VK API via libcurl &amp; jansson

## usage
Anonymous access rights:
```vkgrab <USER|GROUP>```
OR
```vkgrab -u USER```
OR
```vkgrab -g GROUP```

Use with your profile rights:
* ```vkgrab -T``` *get token by link*
* ```vkgrab -t TOKEN <USER|GROUP>```


## features
* albums downloading
* wall posts downloading
* documents downloading
* getting attachments from wall posts (like photos, documents or links)
* parsing comments wall
* downloaded files are saved in structured directories
* only screen name is needed

## dependencies
* jansson
* libcurl
* for some operations you'll need user access token, see config.h for details

## windows
1. For windows at first you should [download cygwin](https://cygwin.com/install.html).
2. Pick any repository mirror and continue to packages. Be sure you've chosen **libcurl4, libcurl-devel, wget, cygwin32-gcc-core, gcc-core, git, make, nano**. Let cygwin installer download it's dependencies aswell.
3. Launch cygwin console via desktop link or any other way.
4. Install jansson. Jansson is not presented in cygwin repositories, so you have to compile it by yourself. Just copy/paste this sequence (use shift+insert in terminal):
  ```
  wget http://www.digip.org/jansson/releases/jansson-2.11.tar.gz

  tar xvf jansson-2.11.tar.gz && cd jansson-2.11

  ./configure --prefix=/usr && make && make install

  cd ..
  ```
5. Clone and build vkgrab:
  ```
  git clone https://github.com/Bfgeshka/vkgrab.git

  cd ./vkgrab
  ```
6. There you should configure vkgrab for youself, i.e. edit config.h: ```nano ./config.h```. After finishing, press Ctrl+X and 'y' (for saving changes).
7. Compile now: ```make && make install```
8. If compilation was successfull, you've installed vkgrab! Type ```vkgrab -h``` for getting started.

## building in unix-like os
- edit config.h first!
- ```$ make```
- ```# make install```

