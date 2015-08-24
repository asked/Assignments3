CFLAGS	:= 
LIBS    :=  -lpthread -lrt
SRC		:=  priority_queue.c mts.c
		
OUT 	:= mts

.PHONY: all
all:
	gcc $(CFLAGS) -o $(OUT) -I. $(SRC) $(LIBS)

.PHONY: clean
clean:
	rm mts

