
all:
	$(MAKE) -C src-server
	mv src-server/ekeys .

#	$(MAKE) -C src-client
#	mv src-client/cekeys .
clean:
	rm -f ekeys cekeys src-server/*.o src-client/*.o

