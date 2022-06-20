CC = "gcc"
PROJECT = capturev4l2
SRC = capturev4l2.c

# LIBS = `pkg-config opencv4 --cflags --libs`
$(PROJECT) : $(SRC) client
	$(CC) $(SRC) -o $(PROJECT) $(LIBS)

client : client.c
	$(CC) client.c -o client
clean:
	rm -rf $(PROJECT)
	rm -rf hello.png client
	rm -rf /home/ngoquang/Desktop/workplaces/Learn_CV/demo/hello.png