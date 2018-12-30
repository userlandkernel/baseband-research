libnest_extras.a: nest_client.o nest_server.o pingpong_client.h pingpong_service.h pingpong_server.o
	ar rcs libnest_extras.a nest_client.o nest_server.o pingpong_server.o

nest_client.o: nest_client.c pingpong_client.h

nest_server.o: nest_server.c pingpong_service.h

pingpong_client.h: pingpong.idl
	magpidl4.py -p nicta -i v4nicta_n2 -w 32 -c -h pingpong_client.h pingpong.idl

pingpong_service.h: pingpong.idl
	magpidl4.py -p nicta -i v4nicta_n2 -w 32 --service-headers -h pingpong_service.h pingpong.idl

pingpong_server.c: pingpong.idl
	magpidl4.py -p nicta -i v4nicta_n2 -w 32 -s -h pingpong_server.c pingpong.idl
