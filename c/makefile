SRC=base64.c encode.c decode.c


base64: $(SRC)
	cc -o base64 -g $(SRC)

test:
	./base64    < base64.c > base64.c.encoded
	./base64 -d < base64.c.encoded >base64.c.decoded
	diff -q base64.c.decoded base64.c	

test_encode:
	./base64 <<< "hello"
	base64   <<< "hello"
	./base64 <<< "hello "
	base64   <<< "hello "
	./base64 <<< "hello t"
	base64   <<< "hello t"
	./base64 <<< "hello th"
	base64   <<< "hello th"
	./base64 <<< "hello the"
	base64   <<< "hello the"


test_decode:
	./base64 -d <<< "aGVsbG8K"
	base64   -d <<< "aGVsbG8K"
	./base64 -d <<< "aGVsbG8gCg=="
	base64   -d <<< "aGVsbG8gCg=="
	./base64 -d <<< "aGVsbG8gdAo="
	base64   -d <<< "aGVsbG8gdAo="
	./base64 -d <<< "aGVsbG8gdGgK"
	base64   -d <<< "aGVsbG8gdGgK"
	./base64 -d <<< "aGVsbG8gdGhlCg=="
	base64   -d <<< "aGVsbG8gdGhlCg=="


validate:
	base64    < base64.c | diff - base64.c.encoded
	base64 -d < base64.c.encoded | diff - base64.c



clean:
	rm -fr *.o base64.dSYMM base64
	rm -fr base64.c.*
