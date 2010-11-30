include comp356.mk

BINS=show_maze2d hw4

show_maze2d : show_maze2d.o maze.o
	$(CC) -o $@ $(CFLAGES) $(CPPFLAGS) $^ $(LDFLAGS) -l356
	
hw4 : hw4.o maze.o
	$(CC) -o $@ $(CFLAGES) $(CPPFLAGS) $^ $(LDFLAGS) -l356

clean :
	rm -f *.o $(BINS)
