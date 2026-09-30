CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g -Wunused-result
LDFLAGS = -lcurl -lcjson -luuid

SRCDIR = src
BUILDDIR = build
INCLUDE = include

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
BIN = $(BUILDDIR)/main

all: $(BIN)

$(BIN): $(OBJECTS)
	@mkdir -p $(BUILDDIR)
	$(CC) $(OBJECTS) -o $(BIN) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
