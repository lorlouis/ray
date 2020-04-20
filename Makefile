SOURCE	= main.c raycast.c texture.c
HEADER	= common.h raycast.h texture.h
CC	 = gcc -std=c89 -ansi
FLAGS	 = -g -c -Wall
LFLAGS	 = -lX11 -lm
BUILD_DIR = build
MKDIR_P = mkdir -p

SDL_OUT	= ray_sdl
_SDL_OBJS = main_sdl.o raycast.o texture.o
SDL_OBJS = $(patsubst %,$(BUILD_DIR)/%,$(_SDL_OBJS))


sdl: $(SDL_OBJS)
	$(CC) -g $(SDL_OBJS) -o $(SDL_OUT) $(LFLAGS) -lSDL2

$(BUILD_DIR)/%.o: %.c
	$(MKDIR_P) $(BUILD_DIR)
	$(MKDIR_P) $(BUILD_DIR)/lib
	$(CC) $(FLAGS) $< -o $@


clean:
	rm -rf $(BUILD_DIR)
