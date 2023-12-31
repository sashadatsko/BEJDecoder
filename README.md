# BEJDecoder
## Building and running
```
$ make
$ ./bejdecoder <dictionary> <input bej files>
```
## Example
memory.dict:
```
VersionTag:  0
DictionaryFlags:  0
EntryCount:  377
SchemaVersion:  0xf117f100
DictionarySize:  7810
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| Row       |   Sequence# | Format   | Flags                         | Field String                | Offset    |   Child Count |
+===========+=============+==========+===============================+=============================+===========+===============+
| 0(12)     |           0 | Set      |                               | Memory                      | 1(22)     |            74 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 1(22)     |           0 | Set      |                               | Actions                     | 75(762)   |            13 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 2(32)     |           1 | Array    | ,Permission=Read              | AllowedSpeedsMHz            | 88(892)   |             1 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 3(42)     |           2 | Enum     | Nullable=True,Permission=Read | BaseModuleType              | 89(902)   |            11 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 4(52)     |           3 | Integer  | Nullable=True,Permission=Read | BusWidthBits                | 0(0)      |             0 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 5(62)     |           4 | Integer  | Nullable=True,Permission=Read | CapacityMiB                 | 0(0)      |             0 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 6(72)     |           5 | Integer  | Nullable=True,Permission=Read | DataWidthBits               | 0(0)      |             0 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 7(82)     |           6 | String   | Nullable=True,Permission=Read | Description                 | 0(0)      |             0 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 8(92)     |           7 | String   | Nullable=True,Permission=Read | DeviceID                    | 0(0)      |             0 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 9(102)    |           8 | String   | Nullable=True,Permission=Read | DeviceLocator               | 0(0)      |             0 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+
| 10(112)   |           9 | Enum     | Nullable=True,Permission=Read | ErrorCorrection             | 100(1012) |             4 |
+-----------+-------------+----------+-------------------------------+-----------------------------+-----------+---------------+

...
```
memory.bin:
```
00000000  00 f0 f0 f1 00 00 00 01  00 00 01 4d 01 06 01 08  |...........M....|
00000010  30 01 03 00 00 01 01 0a  30 01 01 40 01 1c 70 01  |0.......0..@..p.|
00000020  01 01 01 02 10 01 10 01  02 01 00 30 01 02 60 09  |...........0..`.|
00000030  01 02 30 01 02 80 0c 01  0e 50 01 0a 46 46 32 33  |..0......P..FF23|
00000040  31 32 33 45 42 00 01 26  00 01 0e 01 02 01 00 30  |123EB..&.......0|
00000050  01 01 00 01 04 30 01 01  00                       |.....0...|
```
### Decoding
```
$ ./bejdecoder example/memory.dict example/memory.bin
```
memory.bin.json:
```
{
    "CapacityMiB": 65536,
    "DataWidthBits": 64,
    "IsRankSpareEnabled": true,
    "AllowedSpeedsMHz": [
        2400,
        3200
    ],
    "DeviceID": "FF23123EB",
    "MemoryLocation": {
        "Channel": 0,
        "Slot": 0
    }
}
```