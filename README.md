# check_unifi
Nagios® checks for UniFi® Wifi Access-Points

This is _not_ an official product of Ubiquiti Networks. UniFi is a registered trademark of Ubiquiti Networks, Inc.

##Development
This software is in an early development phase and will soon be improved to better fit my needs. You are welcome
to contribute code and bug reports. It's quite possible that at least a method of automatic generation of hosts
checks and dependencies will be added.

##Usage
At the moment you have to fetch your device.json file from the UniFi controller manually by downloading
https://{CONTROLLER}/api/s/{SITE}/stat/device for example by using wget or curl. Controller availability should 
be checked using check_http which does that job perfectly.

##FAQs

###Why do you use C++?
I already used libjsoncpp and did not want to dig into a new library to finish this project fast so I decided
to use this one.

###What do I need to build this?
You need a C++11 compliant compiler like G++ (I'm using g++-4.9 for development) and libjsoncpp. On Debian 8 you
should just run
```
apt-get install g++ libjsoncpp-dev make
make
```
