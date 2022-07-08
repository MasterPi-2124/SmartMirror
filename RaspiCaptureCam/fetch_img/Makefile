CC = "gcc"
PROJECT = fetch_img 
SRC = fetch_img.c

# LIBS = `pkg-config opencv4 --cflags --libs`
$(PROJECT) : $(SRC)
	$(CC) $(SRC) -o $(PROJECT) $(LIBS)

clean:
	rm -rf $(PROJECT)