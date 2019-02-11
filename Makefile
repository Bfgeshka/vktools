CC = cc

SRC_COMMON = src/stringutils.c src/curlutils.c src/json.c src/request_gen.c src/os.c src/account.c src/content.c src/content_download.c src/content_messages.c
SRC_GRAB = src/vkgrab.c src/application_vkgrab.c
SRC_OWNERDUMP = src/vkownerdump.c src/application_vkownerdump.c

NAMEGRAB = vkgrab
NAMEOWNERDUMP = vkownerdump
PREFIX = /usr/local

CFLAGS = -O2 -g -Wall -Wextra -Wpedantic --std=c99 -D_DEFAULT_SOURCE -I./include
LDFLAGS := $(shell pkg-config --libs jansson libcurl)

all: clean options config ${NAMEGRAB} ${NAMEOWNERDUMP}

options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${NAMEGRAB}:
	${CC} ${SRC_COMMON} ${SRC_GRAB} ${CFLAGS} ${LDFLAGS} -o ${NAMEGRAB}

${NAMEOWNERDUMP}:
	${CC} ${SRC_COMMON} ${SRC_OWNERDUMP} ${CFLAGS} ${LDFLAGS} -o ${NAMEOWNERDUMP}

config:
	cp -n config.def.h config.h

clean:
	rm -f ${NAMEGRAB}
	rm -f ${NAMEOWNERDUMP}

install:
	cp -i ${NAMEGRAB} ${PREFIX}/bin
	cp -i ${NAMEOWNERDUMP} ${PREFIX}/bin

uninstall:
	rm -i ${PREFIX}/bin/${NAMEGRAB}
	rm -i ${PREFIX}/bin/${NAMEOWNERDUMP}

.PHONY: all install uninstall clean
