# platform detection using OS environment variable for Windows
# and uname output of everthing else
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
else
    PLATFORM := $(shell uname)
endif

SRC_PATH=./src/
CC=gcc
CFLAGS = -g -c -DPLATFORM_$(PLATFORM) -I. -I$(SRC_PATH)
LDFLAGS = -lm -lGLEW -L/usr/lib -L.
TARGET=cadzinho

ifeq ($(PLATFORM),Darwin)
    CFLAGS += -I/usr/local/Cellar/lua/5.4.3/include/lua/
    LDFLAGS += -framework OpenGL
else
    LDFLAGS += -lGL -lGLU
endif

ifeq ($(PLATFORM),Linux)
    CFLAGS += `pkg-config --cflags lua sdl2`
    LDFLAGS += -ldl -lpthread -pthread `pkg-config --libs lua sdl2`
else
    CFLAGS += -I/usr/include/SDL2
    LDFLAGS += `sdl2-config --cflags --libs` -llua
endif

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))

all: $(SRC) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

./obj/%.o: ./src/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ) $(TARGET)
