## Info

I would like to announce USB host library for esp32 S2 compatible with arduino, esp-idf and PIO (when pio will be updated to most recent esp-idf).
I have some working library with simple host example and host mouse example is working too.

I will try to make library more user friendly and maybe in future will add more USB device class (will add CDC for sure).
Here is example log with connected mouse:
```
start
[   575][I][port.cpp:96] init(): USB host setup properly
[   575][I][port.cpp:101] init(): Port is power ON now
[   835][I][mouse.ino:135] port_cb(): PORT event: 1
[   835][I][mouse.ino:139] port_cb(): physical connection detected
[   895][I][pipe.cpp:30] USBHostPipe(): constructor
[   895][I][pipe.cpp:82] init(): Creating pipe

[   895][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 225003
[   900][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 224903
[   901][I][mouse.ino:63] ctrl_pipe_cb(): 

[   913][I][mouse.ino:103] ctrl_pipe_cb(): address set: 2
[   918][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 224923
[   925][I][mouse.ino:98] ctrl_pipe_cb(): set current configuration: 1
[   931][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 224923

[   944][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 224667
[   951][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 224587


Config:
Number of Interfaces: 1
Attributes: 0xa0
Max power: 100 mA

Interface:
bInterfaceNumber: 0
bAlternateSetting: 0
bNumEndpoints: 1
bInterfaceClass: 0x03 (HID)
bInterfaceSubClass: 0x01
bInterfaceProtocol: 0x02

Endpoint:
bEndpointAddress: 0x81
bmAttributes: 0x03
bDescriptorType: 5
wMaxPacketSize: 6
bInterval: 10 ms
[   995][I][pipe.cpp:30] USBHostPipe(): constructor
[   999][I][pipe.cpp:82] init(): Creating pipe

[  1004][I][pipe.cpp:92] init(): EP: 0x3ffc6ecb
[  1008][I][pipe.cpp:93] init(): EP: 7, 10
[  1012][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 218651
[  1019][I][mousepipe.cpp:8] inData(): EP: 0x01
[  1023][I][mouse.ino:111] ctrl_pipe_cb(): unknown request handled
[  1029][I][mouse.ino:63] ctrl_pipe_cb(): 
get descriptor 1: 8704
[  1035][I][mouse.ino:89] ctrl_pipe_cb(): HID report map
[ 14405][I][mouse.ino:24] hid_pipe_cb(): 1
[ 14406][I][mouse.ino:25] hid_pipe_cb(): 0
[ 14406][I][mouse.ino:26] hid_pipe_cb(): 0/0/0
[ 14406][I][mouse.ino:27] hid_pipe_cb(): 0

[  3017][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219307
[  3024][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 10200][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 10200][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 1
[ 10200][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 0/0/0
[ 10205][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 10210][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 10217][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 10296][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 10296][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 0
[ 10296][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 0/0/0
[ 10301][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 10306][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 10313][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 12864][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 12864][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 2
[ 12864][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 0/0/0
[ 12869][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 12874][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 12881][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 13048][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 13048][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 0
[ 13048][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 0/0/0
[ 13053][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 13058][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 13065][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 14632][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 14633][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 0
[ 14633][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 0/0/0
[ 14638][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 14643][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 14650][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 14664][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 14665][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 0
[ 14665][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 48/16/254
[ 14670][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 14676][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 14683][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 14688][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 14692][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 0
[ 14697][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 71/96/252
[ 14703][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 14709][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 14715][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 14720][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 14725][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 0
[ 14730][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 3/32/0
[ 14736][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0

[ 14741][I][pipe.cpp:241] allocateIRP(): Creating new IRP, free memory: 219315
[ 14748][I][mousepipe.cpp:8] inData(): EP: 0x01
[ 14760][I][mouse.ino:24] hid_pipe_cb(): HID REPORT ID: 1
[ 14761][I][mouse.ino:25] hid_pipe_cb(): Mouse buttons: 0
[ 14763][I][mouse.ino:26] hid_pipe_cb(): X/Y axes(raw bytes): 0/0/0
[ 14769][I][mouse.ino:27] hid_pipe_cb(): Mouse wheel: 0
```
