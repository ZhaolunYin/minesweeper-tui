CC = cc
OBJECTS = draw.o grid.o main.o ui.o
HEADERS = draw.h grid.h ui.h
TARGET = minesweeper-tui
CFLAGS = $(shell pkg-config --cflags ncurses)
LDLIBS = $(shell pkg-config --libs ncurses)

.PHONY: clean run
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDLIBS)
clean:
	@rm -f $(TARGET) $(OBJECTS) 
run: $(TARGET)
	./$(TARGET)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<
