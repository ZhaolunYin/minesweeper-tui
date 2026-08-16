CC = cc
OBJECTS = draw.o grid.o main.o ui.o
HEADERS = draw.h grid.h ui.h
TARGET = minesweeper
FLAGS = $(shell pkg-config --cflags --libs ncurses)

.PHONY: clean run
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(FLAGS)
clean:
	@rm -f $(TARGET) $(OBJECTS)
run: $(TARGET)
	./$(TARGET)
$(OBJECTS): $(HEADERS)
