# UCI for Windows
A port of OpenWRT UCI utilities and libraries (http://git.openwrt.org/project/uci.git) for use on Windows systems with MinGW compilers.

# Building
```
mkdir build
cd build
cmake .. -G "Unix Makefiles"
make all
```

# Notes
UCI configuration and temporary files are stored respectively in the %USERPROFILE%\\.uci\\config and %USERPROFILE\\.uci\\temp folders by default (e.g. C:\\Users\\Username\\.uci\\config).

Tested with MinGW 4.9.3.