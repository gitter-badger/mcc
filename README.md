MCC
==========

MCC is a MineCraft Client API written in C. Its purpose is to enable automation in Minecraft with high level functions and event-driven responses.

[![Build Status](https://travis-ci.org/NosotrosNueces/mcc.svg?branch=travis)](https://travis-ci.org/NosotrosNueces/mcc)

## Documentation and Resources ##
[wiki.vg](http://wiki.vg)

[mcc.lf.lc](http://mcc.lf.lc)

## Packet Format Guide ##

Structs can be serialized to bytes by defining a format string for the packet serializer. The following table describes the format guidelines.

| fmt | type     |
| --- | -------- |
| `b` | 1 byte   |
| `h` | 2 bytes  |
| `w` | 4 bytes  |
| `l` | 8 bytes  |
| `q` | 16 bytes |
| `s` | string   |
| `v` | varint32 |
| `*` | array    |

Note: the array format character is a prefix to other types (e.g. `*b` for a char array).
