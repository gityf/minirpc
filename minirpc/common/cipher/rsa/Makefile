OBJ=asn1.o os_unix.o common.o wpabuf.o wpa_debug.o rsa.o bignum.o base64.o

all:test_rsa libuser_rsa.a

test_rsa: $(OBJ) test_rsa.o
	gcc -o $@ $^

libuser_rsa.a: $(OBJ) user_rsa.o
	ar rus $@ $^

%.o:%.c
	gcc -DCONFIG_USE_INTTYPES_H -DCONFIG_INTERNAL_LIBTOMMATH -g -c $<

key:
	openssl genrsa -out privkey 464
	openssl rsa -in privkey -pubout -out pubkey

clean:
	rm -rf *.o test_rsa libuser_rsa.a
