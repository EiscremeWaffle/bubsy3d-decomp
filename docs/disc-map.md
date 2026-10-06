# USA Disc Directory and Executable Map

Generated from the local extracted disc by `python -m tools.map_disc --write-module-maps --write-documentation`.

- Files: 446
- Directories including root: 40
- Extracted file bytes: 637,330,169
- Header-recognized executable modules: 22
- Combined executable payload bytes: 15,818,752
- Verified startup code bytes: 3,784
- Remaining executable payload bytes, code/data unresolved: 15,814,968

This is a complete inventory of this extraction, not proof of an unmodified original-release disc
or a complete function/code map. No file contents or game instruction bytes are published here.

[Full file fingerprints and format evidence](../config/disc-map.json)
[Executable fingerprints](../config/executable-map.json)
[Address coverage with file offsets](../config/executable-coverage.json)

## Interpretation

- Level directories, COMMON, MENU, MOVIES, and XA group files by observed location; their roles are filename-based hints.
- The extraction contains L0 through L20 except L3. Do not invent L3 or conclude that files are missing without checking the original disc.
- HOG, TZP, DUM, STR, and other opaque resources are inventoried but their internal records/compression are not mapped.
- TIM and VAB magic identifies candidates; their complete image/audio structures are not validated.
- Only the listed executables contain the PS-X EXE signature anywhere in the scanned files. Raw or compressed code without it may still exist.
- Every executable loads at the same base address. These are separate module images, not one simultaneously resident RAM layout.
- Repeated startup functions are counted per module, not as unique source functions. Everything remains 0% decompiled.

## Executable Modules

End addresses are exclusive. Payload bytes include both code and data and must not be used as the code-progress denominator.

| Disc path | Loaded start | Loaded end | Entry point | Payload bytes | Verified startup bytes |
| --- | --- | --- | --- | ---: | ---: |
| `L0/L0.EXE` | 0x80010000 | 0x800BE800 | 0x8004EB64 | 714752 | 172 |
| `L1/L1.EXE` | 0x80010000 | 0x800C9800 | 0x8005130C | 759808 | 172 |
| `L10/L10.EXE` | 0x80010000 | 0x800C7800 | 0x8004F8CC | 751616 | 172 |
| `L11/L11.EXE` | 0x80010000 | 0x800C8800 | 0x800535B8 | 755712 | 172 |
| `L12/L12.EXE` | 0x80010000 | 0x800C9800 | 0x80053CF8 | 759808 | 172 |
| `L13/L13.EXE` | 0x80010000 | 0x800CA800 | 0x80053E04 | 763904 | 172 |
| `L14/L14.EXE` | 0x80010000 | 0x800C9800 | 0x80052B4C | 759808 | 172 |
| `L15/L15.EXE` | 0x80010000 | 0x800C8800 | 0x80053CB4 | 755712 | 172 |
| `L16/L16.EXE` | 0x80010000 | 0x800C5800 | 0x80052F0C | 743424 | 172 |
| `L17/L17.EXE` | 0x80010000 | 0x800B5800 | 0x8004B874 | 677888 | 172 |
| `L18/L18.EXE` | 0x80010000 | 0x800B9000 | 0x8004DAF4 | 692224 | 172 |
| `L19/L19.EXE` | 0x80010000 | 0x800C0000 | 0x8004DDA8 | 720896 | 172 |
| `L2/L2.EXE` | 0x80010000 | 0x800C7000 | 0x80051184 | 749568 | 172 |
| `L20/L20.EXE` | 0x80010000 | 0x800C6800 | 0x800510E0 | 747520 | 172 |
| `L4/L4.EXE` | 0x80010000 | 0x800C5000 | 0x8004F618 | 741376 | 172 |
| `L5/L5.EXE` | 0x80010000 | 0x800C7000 | 0x80050670 | 749568 | 172 |
| `L6/L6.EXE` | 0x80010000 | 0x800C2800 | 0x8004EF60 | 731136 | 172 |
| `L7/L7.EXE` | 0x80010000 | 0x800CB800 | 0x80053BA4 | 768000 | 172 |
| `L8/L8.EXE` | 0x80010000 | 0x800C2000 | 0x8004EC34 | 729088 | 172 |
| `L9/L9.EXE` | 0x80010000 | 0x800C9800 | 0x800535A0 | 759808 | 172 |
| `MENU.EXE` | 0x80010000 | 0x80088800 | 0x80028F48 | 493568 | 172 |
| `SLUS_001.10` | 0x80010000 | 0x80088800 | 0x80028F5C | 493568 | 172 |

## Directory Summary

Counts are direct children, not recursive subtree totals.

| Directory | Direct files | File bytes |
| --- | ---: | ---: |
| `.` | 6 | 19180483 |
| `COMMON` | 10 | 1196522 |
| `L0` | 8 | 8218076 |
| `L1` | 8 | 8513692 |
| `L10` | 8 | 8494156 |
| `L11` | 8 | 10007456 |
| `L12` | 8 | 8480388 |
| `L13` | 8 | 8070992 |
| `L14` | 8 | 8350848 |
| `L15` | 8 | 10942276 |
| `L16` | 9 | 8280682 |
| `L17` | 8 | 7821940 |
| `L18` | 9 | 8022284 |
| `L19` | 8 | 7788940 |
| `L2` | 8 | 10138108 |
| `L20` | 8 | 8082060 |
| `L4` | 8 | 8863716 |
| `L5` | 8 | 10853956 |
| `L6` | 8 | 9381460 |
| `L7` | 8 | 8450120 |
| `L8` | 8 | 10176852 |
| `L9` | 8 | 11116136 |
| `MENU` | 32 | 2310641 |
| `MOVIES` | 32 | 235555553 |
| `XA` | 17 | 41058304 |
| `XA/BOP` | 26 | 17891328 |
| `XA/CONT` | 6 | 5079040 |
| `XA/DOG` | 5 | 4227072 |
| `XA/DRIV` | 10 | 10125312 |
| `XA/GLID` | 10 | 7864320 |
| `XA/HIGH` | 14 | 12288000 |
| `XA/HIT` | 33 | 27033600 |
| `XA/IDLE` | 33 | 35684352 |
| `XA/ITEM` | 18 | 12615680 |
| `XA/SCRE` | 5 | 3473408 |
| `XA/UFO` | 4 | 4128768 |
| `XA/USOL` | 5 | 3801088 |
| `XA/WOOI` | 5 | 3735552 |
| `XA/WOOL` | 10 | 7667712 |
| `XA/WORM` | 3 | 2359296 |

## Every File

SHA256 prefixes are for scanning; full SHA256 and SHA1 values are in the JSON inventory.

### .

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `BUBSY.CCS` | 109439 | disc_authoring_metadata | `2736e1d35244` |
| `DUMMY1.DAT` | 9039872 | unresolved | `9f58bd938b3b` |
| `DUMMY2.DAT` | 9039872 | unresolved | `9f58bd938b3b` |
| `MENU.EXE` | 495616 | psx_executable | `e3620fd866a4` |
| `SLUS_001.10` | 495616 | psx_executable | `604467ef7317` |
| `SYSTEM.CNF` | 68 | boot_configuration | `9efadebfdeaa` |

### COMMON

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `ABCFONT.TIM` | 46084 | tim_candidate | `575c02e8f691` |
| `BUB.TZP` | 218260 | unresolved | `bbdb92d0e290` |
| `BUBSWIM.TZP` | 239572 | unresolved | `f9b420968de7` |
| `EOL.SEP` | 4082 | unresolved | `636e1cdbfd95` |
| `EOL.VB` | 111664 | unresolved | `4d629e931bd3` |
| `EOL.VH` | 21024 | vab_header_candidate | `075d0a56f122` |
| `GUN.TIM` | 576 | tim_candidate | `c2b429992e05` |
| `PLISKIN.HOG` | 97428 | unresolved | `aeb25214b411` |
| `PLISKIN.TZP` | 218260 | unresolved | `2f822d655521` |
| `PLISWIM.TZP` | 239572 | unresolved | `fc118c557003` |

### L0

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L0.EXE` | 716800 | psx_executable | `57d7a1323eaf` |
| `L0.VB` | 441904 | unresolved | `94b3ee75e208` |
| `L0.VH` | 36896 | vab_header_candidate | `65622bf6960a` |
| `L0EOL.STR` | 6430720 | unresolved | `fc58c35c3978` |
| `L0SCR.STR` | 51200 | unresolved | `3e9fd541fe63` |
| `PERM.HOG` | 218036 | unresolved | `8903f28077fc` |
| `SCRIM.TIM` | 66080 | tim_candidate | `ff4dd31a8355` |
| `TRANS.HOG` | 256440 | unresolved | `ef7355a8d8e3` |

### L1

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L1.EXE` | 761856 | psx_executable | `4408daebb1f4` |
| `L1.VB` | 441904 | unresolved | `94b3ee75e208` |
| `L1.VH` | 36896 | vab_header_candidate | `65622bf6960a` |
| `L1EOL.STR` | 6559744 | unresolved | `f32675ca4b0e` |
| `L1SCR.STR` | 51200 | unresolved | `deb50cb2151e` |
| `PERM.HOG` | 290124 | unresolved | `596de7ebeb97` |
| `SCRIM.TIM` | 66080 | tim_candidate | `ff4dd31a8355` |
| `TRANS.HOG` | 305888 | unresolved | `bd7f9eb87a7a` |

### L10

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L10.EXE` | 753664 | psx_executable | `5a64753276fc` |
| `L10.VB` | 494208 | unresolved | `4102f6e526f8` |
| `L10.VH` | 33312 | vab_header_candidate | `c45916209aae` |
| `L10EOL.STR` | 6621184 | unresolved | `da1799c1f8af` |
| `L10SCR.STR` | 51200 | unresolved | `b1c68669da21` |
| `PERM.HOG` | 205376 | unresolved | `93d8c1e29394` |
| `SCRIM.TIM` | 66080 | tim_candidate | `74c6a32e3b50` |
| `TRANS.HOG` | 269132 | unresolved | `b11c014872f0` |

### L11

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L11.EXE` | 757760 | psx_executable | `aa4d595bb31e` |
| `L11.VB` | 433408 | unresolved | `307a0785e883` |
| `L11.VH` | 39968 | vab_header_candidate | `fd1c076c466e` |
| `L11EOL.STR` | 8075264 | unresolved | `ed618c636667` |
| `L11SCR.STR` | 51200 | unresolved | `3fd3503b2bb6` |
| `PERM.HOG` | 364128 | unresolved | `f74d85236e5b` |
| `SCRIM.TIM` | 66080 | tim_candidate | `fc92541bc926` |
| `TRANS.HOG` | 219648 | unresolved | `a5964dfdea7d` |

### L12

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L12.EXE` | 761856 | psx_executable | `3c44d6e1a0f0` |
| `L12.VB` | 494208 | unresolved | `4102f6e526f8` |
| `L12.VH` | 33312 | vab_header_candidate | `c45916209aae` |
| `L12EOL.STR` | 6436864 | unresolved | `44178b34ea68` |
| `L12SCR.STR` | 51200 | unresolved | `18b4f0743024` |
| `PERM.HOG` | 363468 | unresolved | `aa6f75270165` |
| `SCRIM.TIM` | 66080 | tim_candidate | `74c6a32e3b50` |
| `TRANS.HOG` | 273400 | unresolved | `784fba97ea6c` |

### L13

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L13.EXE` | 765952 | psx_executable | `80b40ed3f6cd` |
| `L13.VB` | 433408 | unresolved | `307a0785e883` |
| `L13.VH` | 39968 | vab_header_candidate | `fd1c076c466e` |
| `L13EOL.STR` | 6066176 | unresolved | `1d94c798f7af` |
| `L13SCR.STR` | 51200 | unresolved | `7533c9b2e2a8` |
| `PERM.HOG` | 372484 | unresolved | `7bff9e8f7686` |
| `SCRIM.TIM` | 66080 | tim_candidate | `fc92541bc926` |
| `TRANS.HOG` | 275724 | unresolved | `c13808dd8b85` |

### L14

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L14.EXE` | 761856 | psx_executable | `fb18e81e8cf6` |
| `L14.VB` | 494208 | unresolved | `4102f6e526f8` |
| `L14.VH` | 33312 | vab_header_candidate | `c45916209aae` |
| `L14EOL.STR` | 6313984 | unresolved | `05a4facab345` |
| `L14SCR.STR` | 51200 | unresolved | `3e0c33920b7c` |
| `PERM.HOG` | 345988 | unresolved | `e0f6deb8cb35` |
| `SCRIM.TIM` | 66080 | tim_candidate | `74c6a32e3b50` |
| `TRANS.HOG` | 284220 | unresolved | `ecf5dfcd966b` |

### L15

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L15.EXE` | 757760 | psx_executable | `c78c281dd1f2` |
| `L15.VB` | 433408 | unresolved | `307a0785e883` |
| `L15.VH` | 39968 | vab_header_candidate | `fd1c076c466e` |
| `L15EOL.STR` | 9039872 | unresolved | `9f58bd938b3b` |
| `L15SCR.STR` | 51200 | unresolved | `2e174e422ed7` |
| `PERM.HOG` | 306828 | unresolved | `dfa941c3decc` |
| `SCRIM.TIM` | 66080 | tim_candidate | `fc92541bc926` |
| `TRANS.HOG` | 247160 | unresolved | `09b4b2101dda` |

### L16

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L16.EXE` | 745472 | psx_executable | `2da37809ad80` |
| `L16.VB` | 407952 | unresolved | `c5019098300c` |
| `L16.VH` | 28704 | vab_header_candidate | `169985fe4b5b` |
| `L16EOL.STR` | 6170624 | unresolved | `c5ec08450037` |
| `L16SCR.STR` | 51200 | unresolved | `2f1bb4e4ca99` |
| `LOG.BAK` | 21002 | unresolved | `f260e38631b8` |
| `PERM.HOG` | 453484 | unresolved | `5a04570dc85d` |
| `SCRIM.TIM` | 66080 | tim_candidate | `7cb86fa98ea6` |
| `TRANS.HOG` | 336164 | unresolved | `8bdb2676001c` |

### L17

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L17.EXE` | 679936 | psx_executable | `fd64d55e9125` |
| `L17.VB` | 438816 | unresolved | `dd06c714cf88` |
| `L17.VH` | 40480 | vab_header_candidate | `1ef352e8d9aa` |
| `L17EOL.STR` | 6170624 | unresolved | `c5ec08450037` |
| `L17SCR.STR` | 51200 | unresolved | `37125c2dbda5` |
| `PERM.HOG` | 199760 | unresolved | `7ef8135a1c0a` |
| `SCRIM.TIM` | 66080 | tim_candidate | `d84dfc9c8943` |
| `TRANS.HOG` | 175044 | unresolved | `c16c9c245153` |

### L18

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L18.EXE` | 694272 | psx_executable | `ce17d2c330ca` |
| `L18.VB` | 364384 | unresolved | `3a9d55f683a8` |
| `L18.VH` | 39968 | vab_header_candidate | `10fab2727660` |
| `L18EOL.STR` | 6170624 | unresolved | `c5ec08450037` |
| `L18SCR.STR` | 40960 | unresolved | `03fe26e5b0ac` |
| `LOG.BAK` | 27696 | unresolved | `29add9562f0a` |
| `PERM.HOG` | 364224 | unresolved | `920ba5ceefb8` |
| `SCRIM.TIM` | 66080 | tim_candidate | `fe9973579cf9` |
| `TRANS.HOG` | 254076 | unresolved | `332570c42e94` |

### L19

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L19.EXE` | 722944 | psx_executable | `041aaa58867c` |
| `L19.VB` | 360560 | unresolved | `ede3421f260e` |
| `L19.VH` | 29216 | vab_header_candidate | `4b3596a7ec64` |
| `L19EOL.STR` | 6170624 | unresolved | `c5ec08450037` |
| `L19SCR.STR` | 51200 | unresolved | `538aed35d4a0` |
| `PERM.HOG` | 191604 | unresolved | `81ecd615c78c` |
| `SCRIM.TIM` | 66080 | tim_candidate | `1bcdb0a07a1f` |
| `TRANS.HOG` | 196712 | unresolved | `d1cd7d3d1acf` |

### L2

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L2.EXE` | 751616 | psx_executable | `47309aa56e83` |
| `L2.VB` | 441904 | unresolved | `94b3ee75e208` |
| `L2.VH` | 36896 | vab_header_candidate | `65622bf6960a` |
| `L2EOL.STR` | 8212480 | unresolved | `c60f848bc818` |
| `L2SCR.STR` | 51200 | unresolved | `a97b48fa4905` |
| `PERM.HOG` | 296412 | unresolved | `3da2b8b4539c` |
| `SCRIM.TIM` | 66080 | tim_candidate | `ff4dd31a8355` |
| `TRANS.HOG` | 281520 | unresolved | `9228ef527079` |

### L20

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L20.EXE` | 749568 | psx_executable | `dd4d50698723` |
| `L20.VB` | 446304 | unresolved | `9965a60e1df9` |
| `L20.VH` | 29728 | vab_header_candidate | `8a1da3b5af76` |
| `L20EOL.STR` | 6144000 | unresolved | `9d2343140548` |
| `L20SCR.STR` | 51200 | unresolved | `a5875a290fd0` |
| `PERM.HOG` | 326736 | unresolved | `8c948cfd66a3` |
| `SCRIM.TIM` | 66080 | tim_candidate | `1bcdb0a07a1f` |
| `TRANS.HOG` | 268444 | unresolved | `ab5cae6434a0` |

### L4

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L4.EXE` | 743424 | psx_executable | `f0ece2d10341` |
| `L4.VB` | 460144 | unresolved | `3372f3e40c65` |
| `L4.VH` | 31264 | vab_header_candidate | `bf5ea916e85e` |
| `L4EOL.STR` | 6866944 | unresolved | `8d333609be6e` |
| `L4SCR.STR` | 51200 | unresolved | `1d2e4727c0cc` |
| `PERM.HOG` | 296364 | unresolved | `f01a37b122ef` |
| `SCRIM.TIM` | 66080 | tim_candidate | `08de2402fd9b` |
| `TRANS.HOG` | 348296 | unresolved | `2fd050a751be` |

### L5

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L5.EXE` | 751616 | psx_executable | `1ad58443a13a` |
| `L5.VB` | 449952 | unresolved | `c13196271769` |
| `L5.VH` | 28192 | vab_header_candidate | `1a6fc45f4847` |
| `L5EOL.STR` | 8998912 | unresolved | `f0f78d066355` |
| `L5SCR.STR` | 51200 | unresolved | `76d1f5952b85` |
| `PERM.HOG` | 238764 | unresolved | `59e247315f81` |
| `SCRIM.TIM` | 66080 | tim_candidate | `6dbddbba63e4` |
| `TRANS.HOG` | 269240 | unresolved | `3d90be543f1b` |

### L6

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L6.EXE` | 733184 | psx_executable | `1a0efe40c4c7` |
| `L6.VB` | 460144 | unresolved | `3372f3e40c65` |
| `L6.VH` | 31264 | vab_header_candidate | `bf5ea916e85e` |
| `L6EOL.STR` | 7419904 | unresolved | `ae1ceb1f0a4f` |
| `L6SCR.STR` | 51200 | unresolved | `94d6453f8588` |
| `PERM.HOG` | 293272 | unresolved | `1287bbacf70f` |
| `SCRIM.TIM` | 66080 | tim_candidate | `08de2402fd9b` |
| `TRANS.HOG` | 326412 | unresolved | `f1b2a2a9149a` |

### L7

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L7.EXE` | 770048 | psx_executable | `87f0c68e4302` |
| `L7.VB` | 449952 | unresolved | `c13196271769` |
| `L7.VH` | 28192 | vab_header_candidate | `1a6fc45f4847` |
| `L7EOL.STR` | 6512640 | unresolved | `4c52ba7095fa` |
| `L7SCR.STR` | 51200 | unresolved | `fa8ed9726c32` |
| `PERM.HOG` | 286088 | unresolved | `0770b0392fe2` |
| `SCRIM.TIM` | 66080 | tim_candidate | `08de2402fd9b` |
| `TRANS.HOG` | 285920 | unresolved | `d6625dca98b7` |

### L8

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L8.EXE` | 731136 | psx_executable | `ace2fef99cb5` |
| `L8.VB` | 460144 | unresolved | `3372f3e40c65` |
| `L8.VH` | 31264 | vab_header_candidate | `bf5ea916e85e` |
| `L8EOL.STR` | 8220672 | unresolved | `a83d0add176b` |
| `L8SCR.STR` | 51200 | unresolved | `381fd1dff735` |
| `PERM.HOG` | 289800 | unresolved | `c1a5493548cd` |
| `SCRIM.TIM` | 66080 | tim_candidate | `08de2402fd9b` |
| `TRANS.HOG` | 326556 | unresolved | `237b852af0b0` |

### L9

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `L9.EXE` | 761856 | psx_executable | `dfc8fd7d8cbc` |
| `L9.VB` | 449952 | unresolved | `c13196271769` |
| `L9.VH` | 28192 | vab_header_candidate | `1a6fc45f4847` |
| `L9EOL.STR` | 9175040 | unresolved | `9f9cccef5ccc` |
| `L9SCR.STR` | 51200 | unresolved | `fb7eb5514d92` |
| `PERM.HOG` | 303660 | unresolved | `405906e3bb58` |
| `SCRIM.TIM` | 66080 | tim_candidate | `08de2402fd9b` |
| `TRANS.HOG` | 280156 | unresolved | `a910640b5941` |

### MENU

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `2PLAY1.DUM` | 51200 | unresolved | `905fe907d8de` |
| `2PLAY2.DUM` | 51200 | unresolved | `905fe907d8de` |
| `2PLAY3.DUM` | 51200 | unresolved | `905fe907d8de` |
| `2PLAYSCR.STR` | 51200 | unresolved | `905fe907d8de` |
| `BLU1.DUM` | 51200 | unresolved | `b4bf0e3cf2a2` |
| `BLU2.DUM` | 51200 | unresolved | `b4bf0e3cf2a2` |
| `BLU3.DUM` | 51200 | unresolved | `b4bf0e3cf2a2` |
| `BLUPRINT.STR` | 51200 | unresolved | `b4bf0e3cf2a2` |
| `CONTDRIV.STR` | 51200 | unresolved | `6d454f4ffd76` |
| `CONTDUCK.STR` | 51200 | unresolved | `f5b7916a93e8` |
| `CONTOLD.STR` | 51200 | unresolved | `ae392768c34b` |
| `CONTSTAN.STR` | 51200 | unresolved | `df25d0efba86` |
| `CREDIT1.STR` | 51200 | unresolved | `d38bd84a07c9` |
| `CREDIT2.STR` | 51200 | unresolved | `004372dbcc73` |
| `CREDIT3.STR` | 51200 | unresolved | `f30be7291e18` |
| `LDSV1.DUM` | 51200 | unresolved | `d559a2a13bd2` |
| `LDSV2.DUM` | 51200 | unresolved | `d559a2a13bd2` |
| `LDSV3.DUM` | 51200 | unresolved | `d559a2a13bd2` |
| `LDSVSCR.STR` | 51200 | unresolved | `d559a2a13bd2` |
| `LEVSEL1.DUM` | 51200 | unresolved | `998f41a5a767` |
| `LEVSEL2.DUM` | 51200 | unresolved | `998f41a5a767` |
| `LEVSEL3.DUM` | 51200 | unresolved | `998f41a5a767` |
| `LEVSELEK.STR` | 51200 | unresolved | `998f41a5a767` |
| `MENU.SEP` | 7393 | unresolved | `58bf0172500f` |
| `MENU.VB` | 154176 | unresolved | `01938319f80d` |
| `MENU.VH` | 15392 | vab_header_candidate | `7d9c5196b065` |
| `MHOG1.HOG` | 390228 | unresolved | `91b62c37310c` |
| `MHOG2.HOG` | 514076 | unresolved | `48a94d968f4a` |
| `MICON1.TIM` | 192 | tim_candidate | `25739bd8c125` |
| `MICON2.TIM` | 192 | tim_candidate | `571d47fe37b6` |
| `MICON3.TIM` | 192 | tim_candidate | `94242eb62180` |
| `OPT1.DUM` | 51200 | unresolved | `1a65f8d11ff2` |

### MOVIES

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `ACCOLADE.STR` | 4159488 | unresolved | `5171e970bcca` |
| `BUBQUIT.STR` | 30720 | unresolved | `6ccca0cde7d5` |
| `BUBQUIT1.DUM` | 30720 | unresolved | `6ccca0cde7d5` |
| `BUBQUIT2.DUM` | 30720 | unresolved | `6ccca0cde7d5` |
| `CDLOAD.STR` | 51200 | unresolved | `daa8237ee628` |
| `CDLOAD1.DUM` | 51200 | unresolved | `daa8237ee628` |
| `CDLOAD2.DUM` | 51200 | unresolved | `daa8237ee628` |
| `CHAPLIN.STR` | 1669120 | unresolved | `58849be98a9b` |
| `DROWN.STR` | 2275328 | unresolved | `ab246ac92d0b` |
| `EIDETIC.STR` | 3092480 | unresolved | `7f4ff1ea264e` |
| `ELECTRIC.STR` | 2226176 | unresolved | `62ba15db1b03` |
| `EXPLODE.STR` | 2193408 | unresolved | `7146fb2f0957` |
| `FACEFALL.STR` | 1228800 | unresolved | `862308035565` |
| `FALLING.STR` | 2603008 | unresolved | `e15b78ccf85a` |
| `FISHBOWL.STR` | 6150144 | unresolved | `a81c3cb25ca6` |
| `INTRO.STR` | 62447616 | unresolved | `c15e138f1eeb` |
| `JUGGLE.STR` | 6723584 | unresolved | `81af2ee18466` |
| `LOG.BAK` | 21217 | unresolved | `1c9c19fd6af0` |
| `OUTRO1.STR` | 52508672 | unresolved | `fb813a3c4c71` |
| `OUTRO2.STR` | 46899200 | unresolved | `4a969b17d4e0` |
| `PADDLE.STR` | 4530176 | unresolved | `c01da6c65b76` |
| `PUDDLED.STR` | 1679360 | unresolved | `d858458e034e` |
| `PUNCTURE.STR` | 2340864 | unresolved | `cf01f4b03781` |
| `RAIN.STR` | 6150144 | unresolved | `354dc3e19018` |
| `REMOTE.STR` | 6000640 | unresolved | `0bb12cb4df47` |
| `ROCKET.STR` | 3522560 | unresolved | `2037bad3b4c0` |
| `SLIMED.STR` | 2291712 | unresolved | `56e9958af38f` |
| `SPIN.STR` | 2013184 | unresolved | `2fd61f4b6040` |
| `TOPHAT.STR` | 6498304 | unresolved | `d879dc432651` |
| `UFO.STR` | 2254848 | unresolved | `f4adb2d6dfde` |
| `UWDEATH1.STR` | 2129920 | unresolved | `cc897d071066` |
| `UWDEATH2.STR` | 1699840 | unresolved | `9d5e216d29c6` |

### XA

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `Q1.XA` | 4161536 | unresolved | `d5eff807dc27` |
| `Q10.XA` | 2981888 | unresolved | `39972a675804` |
| `Q11.XA` | 1802240 | unresolved | `4991c0c02211` |
| `Q12.XA` | 1769472 | unresolved | `acd08cf23917` |
| `Q13.XA` | 819200 | unresolved | `36f27064d9d0` |
| `Q14.XA` | 3407872 | unresolved | `9e9221b60e3f` |
| `Q15.XA` | 2588672 | unresolved | `15c96b4f1ba8` |
| `Q16.XA` | 589824 | unresolved | `331a34cd38a4` |
| `Q17.XA` | 1114112 | unresolved | `441bb4af6665` |
| `Q2.XA` | 2392064 | unresolved | `6d213743c258` |
| `Q3.XA` | 3178496 | unresolved | `22cda4854bb6` |
| `Q4.XA` | 2916352 | unresolved | `9956fd655d5e` |
| `Q5.XA` | 2031616 | unresolved | `37c1e2407c24` |
| `Q6.XA` | 2195456 | unresolved | `8e1a06ea24b5` |
| `Q7.XA` | 2916352 | unresolved | `783cf9db7ea7` |
| `Q8.XA` | 2949120 | unresolved | `5263c6a57b38` |
| `Q9.XA` | 3244032 | unresolved | `481d19d775e8` |

### XA/BOP

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `BOP0.XA` | 458752 | unresolved | `ddca1b490e53` |
| `BOP1.XA` | 688128 | unresolved | `6c34f8fb3d97` |
| `BOP10.XA` | 753664 | unresolved | `455d0cad64c8` |
| `BOP11.XA` | 425984 | unresolved | `2f548536e9fa` |
| `BOP12.XA` | 393216 | unresolved | `1f2a0d724071` |
| `BOP13.XA` | 983040 | unresolved | `2b6e1c962834` |
| `BOP14.XA` | 851968 | unresolved | `8abbf55c79d9` |
| `BOP15.XA` | 786432 | unresolved | `d4a0a51fb4df` |
| `BOP16.XA` | 425984 | unresolved | `2f1611183354` |
| `BOP17.XA` | 327680 | unresolved | `d26788f31d3b` |
| `BOP18.XA` | 688128 | unresolved | `9bf639698454` |
| `BOP19.XA` | 786432 | unresolved | `0f2954438ad1` |
| `BOP2.XA` | 589824 | unresolved | `212cebfd2ee3` |
| `BOP20.XA` | 622592 | unresolved | `3862b9bafd76` |
| `BOP21.XA` | 1212416 | unresolved | `06310e00c891` |
| `BOP22.XA` | 655360 | unresolved | `c1d95a0ebdc0` |
| `BOP23.XA` | 983040 | unresolved | `2453a0acb77b` |
| `BOP24.XA` | 655360 | unresolved | `48bfa15fd8bf` |
| `BOP25.XA` | 819200 | unresolved | `265d62c9d99b` |
| `BOP3.XA` | 753664 | unresolved | `0681dda140d1` |
| `BOP4.XA` | 753664 | unresolved | `a39f34b00f47` |
| `BOP5.XA` | 557056 | unresolved | `ea4ff3669f8f` |
| `BOP6.XA` | 753664 | unresolved | `aa8314014d6b` |
| `BOP7.XA` | 1081344 | unresolved | `ce7eb062531a` |
| `BOP8.XA` | 458752 | unresolved | `8c94d85ae621` |
| `BOP9.XA` | 425984 | unresolved | `f83284c800e0` |

### XA/CONT

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `CONT0.XA` | 655360 | unresolved | `9c2259ea4987` |
| `CONT1.XA` | 688128 | unresolved | `5d8a6cfc86f2` |
| `CONT2.XA` | 851968 | unresolved | `8e3dfe9b20cd` |
| `CONT3.XA` | 655360 | unresolved | `3838267dd8c0` |
| `CONT4.XA` | 1146880 | unresolved | `bd6cbad9f92d` |
| `CONT5.XA` | 1081344 | unresolved | `aa3fc46d387d` |

### XA/DOG

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `DOG0.XA` | 851968 | unresolved | `abaac7670bcb` |
| `DOG1.XA` | 622592 | unresolved | `8cc6dfcb4ba7` |
| `DOG2.XA` | 688128 | unresolved | `cfff89d4ac15` |
| `DOG3.XA` | 819200 | unresolved | `5b205f7ba49c` |
| `DOG4.XA` | 1245184 | unresolved | `65179960c2c3` |

### XA/DRIV

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `DRIV0.XA` | 753664 | unresolved | `4d41fb083fea` |
| `DRIV1.XA` | 1736704 | unresolved | `a3d4f4c3b5e9` |
| `DRIV2.XA` | 753664 | unresolved | `b033a3f2d65c` |
| `DRIV3.XA` | 884736 | unresolved | `42f145cec498` |
| `DRIV4.XA` | 1048576 | unresolved | `2744470c7afb` |
| `DRIV5.XA` | 524288 | unresolved | `7028bda9d6f6` |
| `DRIV6.XA` | 524288 | unresolved | `3343f6fed4a7` |
| `DRIV7.XA` | 557056 | unresolved | `a0f9a4218057` |
| `DRIV8.XA` | 1867776 | unresolved | `46c94f4e079a` |
| `DRIV9.XA` | 1474560 | unresolved | `4f187f8f063b` |

### XA/GLID

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `GLID0.XA` | 819200 | unresolved | `96462415f4d5` |
| `GLID1.XA` | 753664 | unresolved | `ee12e7e1a107` |
| `GLID2.XA` | 851968 | unresolved | `91831674f9d1` |
| `GLID3.XA` | 720896 | unresolved | `d0279a622387` |
| `GLID4.XA` | 851968 | unresolved | `34782915cba5` |
| `GLID5.XA` | 786432 | unresolved | `3dd46836777a` |
| `GLID6.XA` | 720896 | unresolved | `6a0d75ed70ea` |
| `GLID7.XA` | 884736 | unresolved | `d8430deb2ecc` |
| `GLID8.XA` | 655360 | unresolved | `b9bd23f97ff0` |
| `GLID9.XA` | 819200 | unresolved | `84ed40b066f2` |

### XA/HIGH

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `HIGH0.XA` | 720896 | unresolved | `de8ae06895c4` |
| `HIGH1.XA` | 884736 | unresolved | `7bfe869daa23` |
| `HIGH10.XA` | 983040 | unresolved | `11832ecd692e` |
| `HIGH11.XA` | 720896 | unresolved | `fcbdc547f8ad` |
| `HIGH12.XA` | 1081344 | unresolved | `49cd41340f4f` |
| `HIGH13.XA` | 917504 | unresolved | `b9d32a4b859f` |
| `HIGH2.XA` | 720896 | unresolved | `45a5bebb53a1` |
| `HIGH3.XA` | 753664 | unresolved | `3fe77f848f62` |
| `HIGH4.XA` | 819200 | unresolved | `ffd32fd79060` |
| `HIGH5.XA` | 720896 | unresolved | `2d2884de8588` |
| `HIGH6.XA` | 1015808 | unresolved | `4ec595b4ed64` |
| `HIGH7.XA` | 983040 | unresolved | `63bd24544580` |
| `HIGH8.XA` | 983040 | unresolved | `8b0f06f4c054` |
| `HIGH9.XA` | 983040 | unresolved | `ad80257ae1cc` |

### XA/HIT

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `HIT0.XA` | 884736 | unresolved | `0530f3501e19` |
| `HIT1.XA` | 557056 | unresolved | `fe0d1728a5d7` |
| `HIT10.XA` | 851968 | unresolved | `67fa607fe98e` |
| `HIT11.XA` | 622592 | unresolved | `73697ecb9820` |
| `HIT12.XA` | 753664 | unresolved | `6b2d3a3dd23c` |
| `HIT13.XA` | 884736 | unresolved | `f538d12d4aa4` |
| `HIT14.XA` | 851968 | unresolved | `fd22d7bd33bb` |
| `HIT15.XA` | 524288 | unresolved | `76303851ab6b` |
| `HIT16.XA` | 884736 | unresolved | `06d92e30a0c7` |
| `HIT17.XA` | 851968 | unresolved | `d23d7064d5d2` |
| `HIT18.XA` | 720896 | unresolved | `b6bdc4092ac8` |
| `HIT19.XA` | 983040 | unresolved | `25d8a0c0a8d5` |
| `HIT2.XA` | 983040 | unresolved | `3015780a0492` |
| `HIT20.XA` | 950272 | unresolved | `e34d1aa7347a` |
| `HIT21.XA` | 753664 | unresolved | `d2b0a1cca174` |
| `HIT22.XA` | 819200 | unresolved | `49debbfd7722` |
| `HIT23.XA` | 688128 | unresolved | `ebd1877f2a30` |
| `HIT24.XA` | 917504 | unresolved | `a17fcaa2ab81` |
| `HIT25.XA` | 884736 | unresolved | `baa31667e479` |
| `HIT26.XA` | 851968 | unresolved | `658329592168` |
| `HIT27.XA` | 819200 | unresolved | `a0973aee0514` |
| `HIT28.XA` | 819200 | unresolved | `b38aa42d233f` |
| `HIT29.XA` | 688128 | unresolved | `4d2f29b2502f` |
| `HIT3.XA` | 917504 | unresolved | `7f68f5c6e05d` |
| `HIT30.XA` | 1015808 | unresolved | `6fa07e61c5c5` |
| `HIT31.XA` | 1179648 | unresolved | `9a6c8ecd5a0a` |
| `HIT32.XA` | 786432 | unresolved | `db3bd7c35a5c` |
| `HIT33.XA` | 950272 | unresolved | `22bd7e5ca147` |
| `HIT34.XA` | 917504 | unresolved | `4474231dabb8` |
| `HIT35.XA` | 491520 | unresolved | `0538607ffd35` |
| `HIT4.XA` | 851968 | unresolved | `14b8379a1646` |
| `HIT5.XA` | 425984 | unresolved | `37bd4cff7c4c` |
| `HIT6.XA` | 950272 | unresolved | `87baad8bd41f` |

### XA/IDLE

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `IDLE0.XA` | 1277952 | unresolved | `a9ef129a6dcc` |
| `IDLE1.XA` | 1048576 | unresolved | `54a58c3c0dcd` |
| `IDLE10.XA` | 688128 | unresolved | `a64140b87d43` |
| `IDLE11.XA` | 786432 | unresolved | `5fcf937abdb4` |
| `IDLE12.XA` | 1245184 | unresolved | `aee5936ff0f2` |
| `IDLE13.XA` | 622592 | unresolved | `a083911a6d11` |
| `IDLE14.XA` | 1015808 | unresolved | `700d6b3bd496` |
| `IDLE15.XA` | 1015808 | unresolved | `e89aff411be4` |
| `IDLE16.XA` | 1048576 | unresolved | `94d490e73248` |
| `IDLE17.XA` | 950272 | unresolved | `96b787b7a8e2` |
| `IDLE18.XA` | 983040 | unresolved | `83954eb96558` |
| `IDLE19.XA` | 950272 | unresolved | `fa3c54d8a448` |
| `IDLE2.XA` | 917504 | unresolved | `17c22d74e630` |
| `IDLE20.XA` | 950272 | unresolved | `8ad23132104a` |
| `IDLE21.XA` | 1310720 | unresolved | `ded63df38e1b` |
| `IDLE22.XA` | 819200 | unresolved | `5711f9bb0656` |
| `IDLE23.XA` | 851968 | unresolved | `bdc2c24802c8` |
| `IDLE24.XA` | 720896 | unresolved | `b9eb662358c1` |
| `IDLE25.XA` | 1835008 | unresolved | `a3e86c9d9b4e` |
| `IDLE26.XA` | 884736 | unresolved | `c22bebf77374` |
| `IDLE27.XA` | 720896 | unresolved | `b9eeabf33f36` |
| `IDLE28.XA` | 786432 | unresolved | `f8f551b4680c` |
| `IDLE29.XA` | 720896 | unresolved | `8252aecd0b72` |
| `IDLE3.XA` | 983040 | unresolved | `40e329a39cbc` |
| `IDLE30.XA` | 688128 | unresolved | `ca03940ad241` |
| `IDLE31.XA` | 753664 | unresolved | `e77ec2b4c16a` |
| `IDLE32.XA` | 851968 | unresolved | `91c783f19226` |
| `IDLE33.XA` | 524288 | unresolved | `7c9bb2591d74` |
| `IDLE34.XA` | 1835008 | unresolved | `fb69379203c4` |
| `IDLE35.XA` | 917504 | unresolved | `e2371d74b46a` |
| `IDLE36.XA` | 3768320 | unresolved | `149f9f3bdaf0` |
| `IDLE37.XA` | 2195456 | unresolved | `a7af15b4f9f2` |
| `IDLE4.XA` | 1015808 | unresolved | `a56b9cc15350` |

### XA/ITEM

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `ITEM0.XA` | 688128 | unresolved | `90934d802a69` |
| `ITEM1.XA` | 786432 | unresolved | `b55f8114566a` |
| `ITEM10.XA` | 524288 | unresolved | `a6b5f8ce2f46` |
| `ITEM11.XA` | 622592 | unresolved | `e4f9f10824b4` |
| `ITEM12.XA` | 425984 | unresolved | `49bc88b0fcc1` |
| `ITEM13.XA` | 753664 | unresolved | `00e37705ba0b` |
| `ITEM14.XA` | 753664 | unresolved | `e4e6b3ab7213` |
| `ITEM15.XA` | 786432 | unresolved | `78869ba0cc07` |
| `ITEM16.XA` | 983040 | unresolved | `21a68de997da` |
| `ITEM17.XA` | 524288 | unresolved | `9d0a3e6e18e7` |
| `ITEM2.XA` | 655360 | unresolved | `ad1b8e278081` |
| `ITEM3.XA` | 622592 | unresolved | `e6ea8f961cd0` |
| `ITEM4.XA` | 753664 | unresolved | `e2c1f6964840` |
| `ITEM5.XA` | 622592 | unresolved | `cd21d7aeeaad` |
| `ITEM6.XA` | 753664 | unresolved | `f0852bb74ec5` |
| `ITEM7.XA` | 524288 | unresolved | `150236c050f9` |
| `ITEM8.XA` | 720896 | unresolved | `317cbc8b3c3f` |
| `ITEM9.XA` | 1114112 | unresolved | `b630183267b5` |

### XA/SCRE

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `SCRE0.XA` | 655360 | unresolved | `64af64b5bb90` |
| `SCRE1.XA` | 557056 | unresolved | `acc0dac975fd` |
| `SCRE2.XA` | 851968 | unresolved | `5260ad03cdcd` |
| `SCRE3.XA` | 753664 | unresolved | `249b4e6ffbb4` |
| `SCRE4.XA` | 655360 | unresolved | `cf592f23e26d` |

### XA/UFO

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `UFO0.XA` | 688128 | unresolved | `ddc235e2f603` |
| `UFO1.XA` | 1572864 | unresolved | `dd727873529a` |
| `UFO2.XA` | 1146880 | unresolved | `877b37b2cb06` |
| `UFO3.XA` | 720896 | unresolved | `1f9b5b32672c` |

### XA/USOL

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `USOL0.XA` | 819200 | unresolved | `32248e01aad7` |
| `USOL1.XA` | 655360 | unresolved | `417992e0b5e0` |
| `USOL2.XA` | 688128 | unresolved | `6993722e43d7` |
| `USOL3.XA` | 753664 | unresolved | `eebc45ca766b` |
| `USOL4.XA` | 884736 | unresolved | `326c446f2e92` |

### XA/WOOI

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `WOOI0.XA` | 655360 | unresolved | `670126ec122c` |
| `WOOI1.XA` | 786432 | unresolved | `9998b52ee484` |
| `WOOI2.XA` | 688128 | unresolved | `a3b8aeee5e7c` |
| `WOOI3.XA` | 851968 | unresolved | `ac45119a5bc2` |
| `WOOI4.XA` | 753664 | unresolved | `1ea621d79309` |

### XA/WOOL

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `WOOL0.XA` | 819200 | unresolved | `e5668fdc6421` |
| `WOOL1.XA` | 589824 | unresolved | `6cded365cc6e` |
| `WOOL2.XA` | 1015808 | unresolved | `051cdc3ae5b9` |
| `WOOL3.XA` | 917504 | unresolved | `1fd7b15d6cb2` |
| `WOOL4.XA` | 819200 | unresolved | `c294ff61fbcb` |
| `WOOL5.XA` | 393216 | unresolved | `bbdaf3ecf5fe` |
| `WOOL6.XA` | 786432 | unresolved | `ec67eb3082c7` |
| `WOOL7.XA` | 753664 | unresolved | `42e7e74d3a6b` |
| `WOOL8.XA` | 983040 | unresolved | `56f37e2e07bf` |
| `WOOL9.XA` | 589824 | unresolved | `2c31b092d1e7` |

### XA/WORM

| File | Bytes | Format/evidence status | SHA256 prefix |
| --- | ---: | --- | --- |
| `WORM0.XA` | 655360 | unresolved | `29ca3619060a` |
| `WORM1.XA` | 589824 | unresolved | `94fc908dce7e` |
| `WORM2.XA` | 1114112 | unresolved | `3342be9483f0` |
