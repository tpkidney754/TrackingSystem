CC = gcc
host: CC = gcc
bbb:  CC = arm-linux-gnueabihf-gcc
frdm: CC = arm-none-eabi-gcc
host: SZ = size
bbb:  SZ = arm-linux-gnueabihf-size
frdm: SZ = arm-none-eabi-size
include sources.mk
CFLAGS = -Werror -g -O0 -std=c99 -Arch=x86
host: CFLAGS = -Werror -g -O0 -std=c99 -Arch=x86
bbb:  CFLAGS = -Werror -g -O0 -std=c99 -Arch=ARM
frdm:  CFLAGS = -Werror -g3 -O0 -std=c99 -Arch=ARM --specs=nosys.specs \
		 -mcpu=cortex-m0plus -mthumb -fmessage-length=0 -fsigned-char \
		 -ffunction-sections -fdata-sections  -MMD -MP
bbb: override DEFINES += -DBBB
frdm: override DEFINES += -DFRDM
frdm: LDFLAGS = -Xlinker -Map=main.map -T "MKL25Z128xxx4_flash.ld"
OBJS = $(SRCS:.c=.o)
LIBOBJS = $(LIBS:.c=.o)
PREOBJS = $(SRCS:.c=.i)
ASMOBJS = $(SRCS:.c=.S)
OUTPUT = Tracking



.PHONY: size host bbb frdm preprocess asm-file compile-all build clean build-lib %.o %.i %.S

host: $(OBJS)
	$(CC) $(CFLAGS) $(SRCS) $(INCLUDES) -o $(OUTPUT) $(LDFLAGS) $(DEFINES)
	$(SZ) $(OUTPUT)

bbb: $(OBJS)
	$(CC) $(CFLAGS) $(BBBSRCS) $(INCLUDES) -o $(OUTPUT)bbb $(LDFLAGS) $(DEFINES)
	$(SZ) $(OUTPUT)bbb


frdm:
	$(CC) $(CFLAGS) $(FRDMSRCS) $(STARTUP) $(INCLUDES) -o $(OUTPUT)frdm.elf $(LDFLAGS) $(DEFINES) -DFRDM
	$(SZ) $(OUTPUT)frdm.elf

preprocess: $(PREOBJS)

asm-file: $(ASMOBJS)

compile-all:  $(OBJS)

build: host bbb frdm

upload: bbb
	scp $(OUTPUT)bbb root@192.168.7.2:/home/debian/bin/

build-lib: $(LIBOBJS)
	ar rcs libproject1.a $(LIBOBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c $(INCLUDES) -o $@ $(DEFINES)
	$(CC) -M $(CFLAGS) $*.c > $*.d $(INCLUDES)
	if [ ! -d ObjectFiles ]; then mkdir ObjectFiles; fi
	mv $*.o ./ObjectFiles/

frdm-startup: $(OBJSS)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -MMD -MP -c -o $@ "$<"
	#$(CC) -M $(CFLAGS) $*.c > $*.d $(INCLUDES)
	if [ ! -d ObjectFiles ]; then mkdir ObjectFiles; fi
	mv FRDM_Startup_Code/*.o ./ObjectFiles/

%.i: %.c
	$(CC) -E -o $*.i $*.c $(INCLUDES) $(DEFINES)
	if [ ! -d Preprocess ]; then mkdir Preprocess; fi
	mv $*.i ./Preprocess/

%.S: %.c
	$(CC) $(CFLAGS) -S -o $*.S $*.c $(INCLUDES) $(DEFINES)
	if [ ! -d "./Assembly" ]; then mkdir Assembly; fi
	mv $*.S ./Assembly/

clean:
	-rm -f $(OUTPUT)*  main.map *.a ./Preprocess/* ./ObjectFiles/* ./Assembly/* ./*.d ./*/*.d ./*/*.o
