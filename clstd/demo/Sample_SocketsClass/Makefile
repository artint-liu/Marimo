PROJECT_DIR = .
CLSTD_DIR = ../..
LIB_DIR=$(CLSTD_DIR)\Bin

CXX := g++

INCLUDES += -I$(PROJECT_DIR) -I$(CLSTD_DIR)
CPPFLAGS += -D_DEBUG -D_CL_SYSTEM_LINUX -DPOSIX_THREAD -D_CRT_FINDFILE -D_CL_ENABLE_ICONV -fpermissive -std=gnu++11 -fexec-charset=GBK -g
LDFLAGS += -L$(CLSTD_DIR) -lc -lclstd -lpthread
	
OBJS=\
	Sample_Sockets.o
	

	
all:$(OBJS)
#	$(LD) -o test.out $(OBJS) $(LDFLAGS)
	
Sample_Sockets.o : Sample_Sockets.cpp $(CLSTD_DIR)/libclstd.a
	$(CXX) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	-rm $(OBJS)