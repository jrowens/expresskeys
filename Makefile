
all:
	$(MAKE) -C src-server
	mv src-server/expresskeys .

#	$(MAKE) -C src-client
#	mv src-client/expresskeys-conf .
clean:
	rm -f expresskeys expresskeys-conf src-server/*.o src-client/*.o

