PROJECT_DIR = .
CLSTD_DIR = ../..
LIB_DIR=$(CLSTD_DIR)\Bin

INCLUDES += -I$(PROJECT_DIR) -I$(CLSTD_DIR)
CPPFLAGS += -D_DEBUG -D_CL_SYSTEM_LINUX -DPOSIX_THREAD -D_CRT_FINDFILE -D_CL_ENABLE_ICONV -fpermissive -std=gnu++11 -fexec-charset=GBK
CXX := g++

CRT := \
	/usr/lib/x86_64-linux-gnu/crt1.o \
	/usr/lib/x86_64-linux-gnu/crti.o \
	/usr/lib/x86_64-linux-gnu/crtn.o
#	/usr/lib/x86_64-linux-gnu/gcrt1.o \

LDFLAGS += -L$(CLSTD_DIR) -lc -lclstd -lpthread
#LIBSRCS=\
#	clString.cpp
	
OBJS=\
	Sample_String.o
	

	
all:$(OBJS)
#	$(LD) -o test.out $(OBJS) $(LDFLAGS)
	
Sample_String.o : Sample_String.cpp $(CLSTD_DIR)/libclstd.a
	$(CXX) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	-rm $(OBJS)