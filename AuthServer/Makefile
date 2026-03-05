NAME = test

SRC = CAuthSession.cpp \
	ClientPacketHandler.cpp \
	CoreGlobal.cpp \
	CoreTLS.cpp \
	DBConnection.cpp \
	DBConnectionPool.cpp \
	EpollCore.cpp \
	EpollEvent.cpp \
	EpollObject.cpp \
	JobQueue.cpp \
	Listener.cpp \
	main.cpp \
	NetAddress.cpp \
	ReadBuffer.cpp \
	Service.cpp \
	Session.cpp \
	SslCtx.cpp \
	SslObject.cpp \
	ThreadManager.cpp \
	Timer.cpp \
	Utils.cpp \
	WriteBuffer.cpp \
	Protocol.pb.cc

OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.cc=.o)

CXX = g++
CXXFLAGS = #-Wall -Werror -Wextra
LDLIBS = -lhiredis -lpq -lprotobuf -lssl -lcrypto

all : $(NAME)

$(NAME) : $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDLIBS) -o $(NAME)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o : %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean :
	rm -rf $(OBJ) $(COBJ)

fclean : clean
	rm -rf $(NAME)

re : fclean all

.PHONY : all clean fclean re
