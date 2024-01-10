SRC=base64.c encode.c decode.c


base64: $(SRC)
	cc -o base64 -g $(SRC)


test:
	./base64    < base64.c > base64.c.encoded
	./base64 -d < base64.c.encoded >base64.c.decoded
	diff -q base64.c.decoded base64.c	

validate:
	base64    < base64.c | diff - base64.c.encoded
	base64 -d < base64.c.encoded | diff - base64.c


clean:
	rm -fr *.o base64.dSYMM base64
	rm -fr base64.c.*
