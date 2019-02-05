CC = cc

SRC_COMMON = src/stringutils.c src/curlutils.c src/json.c src/request_gen.c src/os.c src/account.c src/content_processing.c src/content_download.c
SRC_GRAB = src/vkgrab.c src/application_vkgrab.c
SRC_OWNERDUMP = src/vkownderdump.c src/application_vkownerdump.c

NAMEGRAB = vkgrab
NAMEOWNERDUMP = vkownerdump
PREFIX = /usr/local

CFLAGS = -O2 -s -Wall -Wextra -Wpedantic --std=c99 -D_DEFAULT_SOURCE -I./include
LDFLAGS := $(shell pkg-config --libs jansson libcurl)

all: clean options ${NAMEGRAB} ${NAMEOWNERDUMP}

options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${NAMEGRAB}:
	cp -n config.def.h config.h
	${CC} ${SRC_COMMON} ${SRC_GRAB} ${CFLAGS} ${LDFLAGS} -o ${NAMEGRAB}

${NAMEOWNERDUMP}:
	cp -n config.def.h config.h
	${CC} ${SRC_COMMON} ${SRC_GRAB} ${CFLAGS} ${LDFLAGS} -o ${NAMEOWNERDUMP}

clean:
	rm -f ${NAMEGRAB}
	rm -f ${NAMEOWNERDUMP}

install:
	cp -i ${NAMEGRAB} ${PREFIX}/bin

uninstall:
	rm -i ${PREFIX}/bin/${NAMEGRAB}

.PHONY: all install uninstall clean
