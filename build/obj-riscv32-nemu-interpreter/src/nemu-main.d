cmd_/home/cyh627/ics2022/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o := unused

source_/home/cyh627/ics2022/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o := src/nemu-main.c

deps_/home/cyh627/ics2022/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o := \
    $(wildcard include/config/target/am.h) \
  /home/cyh627/ics2022/nemu/include/common.h \
    $(wildcard include/config/mbase.h) \
    $(wildcard include/config/msize.h) \
    $(wildcard include/config/isa64.h) \
  /home/cyh627/ics2022/nemu/include/macro.h \
  /home/cyh627/ics2022/nemu/include/debug.h \
  /home/cyh627/ics2022/nemu/include/utils.h \
    $(wildcard include/config/target/native/elf.h) \
  src/./monitor/sdb/sdb.h \

/home/cyh627/ics2022/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o: $(deps_/home/cyh627/ics2022/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o)

$(deps_/home/cyh627/ics2022/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o):
