CC = "gcc"
PROJECT = server
SRC = server.c

# LIBS = `pkg-config opencv4 --cflags --libs`
$(PROJECT) : $(SRC) server.h
	$(CC) $(SRC) -o $(PROJECT) $(LIBS)

clean:
	rm -rf $(PROJECT)