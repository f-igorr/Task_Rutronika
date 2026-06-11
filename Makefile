CC        ?= gcc
CFLAGS    := -Wall -Wextra -Werror -std=c17 
LDLIBS    := -lavcodec -lavformat
LDLIBS    += -lm
CFLAGS    += $(if $(DEBUG), -g -O0 -fsanitize=address -fno-omit-frame-pointer)

DIR_SRC   := src/
DIR_BIN   := bin/

NAME_SRC  := main.c
NAME_APP  := app


SRC       := $(DIR_SRC)$(NAME_SRC)
APP       := $(DIR_BIN)$(NAME_APP)


.PHONY: all clean 


# готовая исполняемый файл
#all: $(APP) | $(DIR_BIN)

all : | $(DIR_BIN)
	$(CC) $(CFLAGS) $(SRC) -o $(APP) $(LDLIBS)


$(DIR_BIN):
	mkdir -p $(DIR_BIN)


clean: 
	rm -rf $(DIR_BIN)