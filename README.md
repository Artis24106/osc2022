# OSC 2022

## Author

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
|`310551086`| `Artis24106` | `杜萬珩` | artis.hh.tu@gmail.com |

## How to build
> I develope on MacOS

### Install Tollchains for MacOS
- [messense/homebrew-macos-cross-toolchains](https://github.com/messense/homebrew-macos-cross-toolchains)

### Build kernel8.img
```
make
```

### Emulate with QEMU
```
make run
```

### Debug with QEMU and GDB
Use port 1234 for remote debugging:
```
make debug
```

I use kali container to debug, and [hugsy/gef](https://github.com/hugsy/gef) is installed for better gdb debugging experence.
In this case:
```
docker exec -it my_kali fish
gdb-multiarch kernel8.elf
gef-remote host.docker.internal:1234
```