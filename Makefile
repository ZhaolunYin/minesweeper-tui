CC = gcc
SRCDIR = src
BUILDDIR = build

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o, $(SRCS))
HEADERS = $(wildcard $(SRCDIR)/*.h)
TARGET = $(BUILDDIR)/minesweeper-tui

CFLAGS = $(shell pkg-config --cflags ncurses) -I$(SRCDIR) -Wall -Wextra -pedantic
LDLIBS = $(shell pkg-config --libs ncurses)

.PHONY: clean run dirs

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDLIBS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)
