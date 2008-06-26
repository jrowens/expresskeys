
all:
	$(MAKE) -C src-server
	mv src-server/expresskeys .

#	$(MAKE) -C src-client
#	mv src-client/cekeys .
clean:
	rm -f expresskeys cekeys src-server/*.o src-client/*.o

