###STM32 LWIP Ethernet UDP experiments

This project is for experementing with ethernet and getting UDP data transfer
working from an Olimex STM32-E407 to a particular target, be it a development
machine, flight computer, or another STM32.


To talk to a board from your development machine you'll need to create a
static IP on your wired ethernet connection (usually eth0) in the same block
as the project (i.e. 10.0.0.xxx). You should probably disconnect from other
networks like wireless.

Try to ping the board first, and if that works procede to using talk.py to see
if data can flow back and forth.
