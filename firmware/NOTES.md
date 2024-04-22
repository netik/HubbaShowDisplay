
read fw:

esptool.py -b 115200 --port /dev/tty.usbserial-0001 read_flash 0x00000 0x400000 original_fw_flash_4M.bin

write fw:

esptool.py -b 115200 --port /dev/tty.usbserial-0001 read_flash 0x00000 0x400000 original_fw_flash_4M.bin

This rev was
```

NodeMCU 3.0.0.0 built on nodemcu-build.com provided by frightanic.com
        branch: release
        commit: 36cbf9f017d356319a6369e299765eedff191154
        release: 
        release DTS: 202402250804
        SSL: false
        build type: integer
        LFS: 0x10000 bytes total capacity
        modules: enduser_setup,file,gpio,net,node,spi,tmr,uart,wifi
 build 2024-04-18 20:45 powered by Lua 5.1.4 on SDK 3.0.1-dev(fce080e)
 ```
 