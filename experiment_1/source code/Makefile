objects = main.o step1.o step2.o step3.o
cc = gcc
hdrs = myXml.h

xml : main.o step1.o step2.o step3.o
	gcc main.o step1.o step2.o step3.o -o xml -lpthread
main.o  : main.c $(hdrs)
	$(cc) -c main.c
step1.o : step1.c $(hdrs)
	$(cc) -c step1.c
step2.o : step2.c $(hdrs)
	$(cc) -c step2.c
step3.o : step3.c $(hdrs)
	$(cc) -c step3.c

.PHONY : clean
clean :
	-rm -f $(objects)