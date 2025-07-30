CC=gcc
CFLAGS=-Wall -Wextra -std=c99
TARGET=winwing_led_test
SOURCE=winwing_led_test.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

.PHONY: clean install
