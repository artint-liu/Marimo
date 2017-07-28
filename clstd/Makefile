PROJECT_DIR = .
LIB_DIR=$(PROJECT_DIR)\Bin

INCLUDES += -I$(PROJECT_DIR)
CPPFLAGS += -D_DEBUG -D_CL_SYSTEM_LINUX -DPOSIX_THREAD -D_CRT_FINDFILE -D_CL_ENABLE_ICONV -std=gnu++11 -fexec-charset=GBK
CXX := g++
#LIBSRCS=\
#	clString.cpp
	
LIBOBJS=\
	clString.o \
	clBuffer.o \
	clFile.o \
	clAllocator.o \
	clColorSpace.o \
	clCompress.o \
	clCrypt.o \
	clfifo.o \
	clGeometry2D.o \
	clImage.o \
	clImageDecoder.o \
	clMatrix.o \
	clMesh.o \
	clPathFile.o \
	clQuaternion.o \
	clRepository.o \
	clstd.o \
	clStock.o \
	clTokens.o \
	clTransform.o \
	clTree.o \
	clUtility.o \
	FloatX.o \
	PrimeNum.o \
	thread/clLocker.o \
	thread/clMessageThread.o \
	thread/clSignal.o \
	thread/clThread.o \
	thread/clThreadQueue.o \
	thread/clTracingLocker.o \
	3D/Camera.o \
	3D/Geometry.o \
	3D/GeoOp.o \
	else/casemap.o \
	else/ecvt.o \
	socket/clSocket.o \
	socket/clSocketClient.o \
	socket/clSocketServer.o
	
	
all: lib

lib:$(LIBOBJS)
	$(AR) crs libclstd.a $(LIBOBJS)
	
%.o : %.cpp
	$(CXX) -c $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $< -o $@
	
clean:
	-rm $(LIBOBJS)
