PROJECT_DIR := $(shell pwd)
CC = gcc
PROM = power_manager_service
OBJ = power_service.o power_manager.o thermal.o

CFLAGS ?= -I$(PROJECT_DIR) -I/$(PROJECT_DIR)/include -I/$(PROJECT_DIR)/include/libxml -lpthread -lxml2 -rdynamic -g -funwind-tables -D_GNU_SOURCE
$(PROM): $(OBJ)
	$(CC) -o $(PROM) $(OBJ) $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(OBJ) $(PROM)

install:
	sudo install -D -m 755 $(PROM) -t /usr/bin/
