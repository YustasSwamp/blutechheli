shelicopter: evtest.o rfcomm-client.o aap_init.o
	gcc $^ -o $@ -lbluetooth -lpthread

evtest.o: evtest.c
	gcc $^ -c -o $@

aap_init.o: aap_init.c
	gcc $^ -c -o $@

rfcomm-client.o: rfcomm-client.c
	gcc $^ -c -o $@

clean:
	rm *.o shelicopter
