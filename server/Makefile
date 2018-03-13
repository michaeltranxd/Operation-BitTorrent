CC = gcc
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter#-Wmissing-declarations

PTHREAD = -pthread

LIBS = libs/
INCLUDES = -I$(LIBS)

OBJS_SERVER_CLIENT = server_client.c
OBJS_INCLUDES = $(LIBS)server.c $(LIBS)utils.c $(LIBS)client.c $(LIBS)file_transfer.c
OBJS_O = server.o utils.o client.o file_transfer.o

EXE_NAME = server_client

server_client:
	$(CC) $(INCLUDES) $(WARNINGS) -o $(EXE_NAME) $(OBJS_INCLUDES) $(OBJS_SERVER_CLIENT) $(PTHREAD)


clean:
	rm -rf $(EXE_NAME)
