CC = cc

SRC_COMMON = src/stringutils.c src/curlutils.c src/json.c src/request_gen.c src/os.c src/account.c src/content_processing.c
SRC_GRAB = src/vkgrab.c src/application_vkgrab.c

NAMEGRAB = vkgrab
PREFIX = /usr/local

CFLAGS = -O2 -Wall -Wextra -Wpedantic --std=c99 -D_DEFAULT_SOURCE -I./include
LDFLAGS := $(shell pkg-config --libs jansson libcurl)

all: clean options ${NAMEGRAB}

options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${NAMEGRAB}:
	cp -n config.def.h config.h
	${CC} ${SRC_COMMON} ${SRC_GRAB} ${CFLAGS} ${LDFLAGS} -o ${NAMEGRAB}

clean:
	rm -f ${NAMEGRAB}

install:
	cp -i ${NAMEGRAB} ${PREFIX}/bin

uninstall:
	rm -i ${PREFIX}/bin/${NAMEGRAB}

.PHONY: all install uninstall clean
