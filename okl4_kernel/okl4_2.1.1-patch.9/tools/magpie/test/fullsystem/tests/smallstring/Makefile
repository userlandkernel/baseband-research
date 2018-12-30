libnest_extras.a: nest_client.o nest_server.o naming_client.h naming_service.h naming_server.o
	ar rcs libnest_extras.a nest_client.o nest_server.o naming_server.o

nest_client.o: nest_client.c naming_client.h

nest_server.o: nest_server.c naming_service.h

naming_client.h: smallstring.idl
	magpidl4.py -p nicta -i v4nicta_n2 -w 32 -c -h naming_client.h smallstring.idl

naming_service.h: smallstring.idl
	magpidl4.py -p nicta -i v4nicta_n2 -w 32 --service-headers -h naming_service.h smallstring.idl

naming_server.c: smallstring.idl
	magpidl4.py -p nicta -i v4nicta_n2 -w 32 -s -h naming_server.c smallstring.idl
