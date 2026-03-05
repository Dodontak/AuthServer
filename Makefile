#======= AuthServer ========#
AUTH_SERVER_DIR = AuthServer
AUTH_SERVER_EXE = $(AUTH_SERVER_DIR)/AuthServer.exe
AUTH_SERVER_SRC_FILE = AuthServer.cpp \
	CAuthSession.cpp \
	ClientPacketHandler.cpp \
	Protocol.pb.cc
AUTH_SERVER_SRC = $(addprefix $(AUTH_SERVER_DIR)/, $(AUTH_SERVER_SRC_FILE))
AUTH_SERVER_OBJ_DIR = $(AUTH_SERVER_DIR)/ObjectFiles
AUTH_SERVER_OBJ_FILE = $(AUTH_SERVER_SRC_FILE:.cpp=.o)
AUTH_SERVER_OBJ_FILE := $(AUTH_SERVER_OBJ_FILE:.cc=.o)
AUTH_SERVER_OBJ = $(addprefix $(AUTH_SERVER_OBJ_DIR)/, $(AUTH_SERVER_OBJ_FILE))
#======= AuthServer ========#

#======= DummyCient ========#
#======= DummyCient ========#

#======= ServerCore ========#
SERVER_CORE_DIR = ServerCore
SERVER_CORE_LIB = $(SERVER_CORE_DIR)/ServerCore.a
SERVER_CORE_SRC_FILE = CoreGlobal.cpp \
	CoreTLS.cpp \
	DBConnection.cpp \
	DBConnectionPool.cpp \
	EpollCore.cpp \
	EpollEvent.cpp \
	EpollObject.cpp \
	JobQueue.cpp \
	Listener.cpp \
	NetAddress.cpp \
	ReadBuffer.cpp \
	Service.cpp \
	Session.cpp \
	SslCtx.cpp \
	SslObject.cpp \
	ThreadManager.cpp \
	Timer.cpp \
	Utils.cpp \
	WriteBuffer.cpp
SERVER_CORE_SRC = $(addprefix $(SERVER_CORE_DIR)/, $(SERVER_CORE_SRC_FILE))
SERVER_CORE_OBJ_DIR = $(SERVER_CORE_DIR)/ObjectFiles
SERVER_CORE_OBJ_FILE = $(SERVER_CORE_SRC_FILE:.cpp=.o)
SERVER_CORE_OBJ = $(addprefix $(SERVER_CORE_OBJ_DIR)/, $(SERVER_CORE_OBJ_FILE))
#======= ServerCore ========#

SERVER_CORE_INC = -I$(SERVER_CORE_DIR)

CXX = g++
CXXFLAGS = #-Wall -Werror -Wextra
LDLIBS = -lhiredis -lpq -lprotobuf -lssl -lcrypto

all : $(AUTH_SERVER_EXE)

$(AUTH_SERVER_EXE) : $(AUTH_SERVER_OBJ_DIR) $(SERVER_CORE_LIB) $(AUTH_SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $(AUTH_SERVER_OBJ) $(SERVER_CORE_LIB) $(LDLIBS) -o $(AUTH_SERVER_EXE)

$(AUTH_SERVER_OBJ_DIR) :
	mkdir -p $@

$(AUTH_SERVER_OBJ_DIR)/%.o : $(AUTH_SERVER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(SERVER_CORE_INC) -o $@ -c $<

$(AUTH_SERVER_OBJ_DIR)/%.o : $(AUTH_SERVER_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(SERVER_CORE_INC) -o $@ -c $<

$(SERVER_CORE_LIB) : $(SERVER_CORE_OBJ_DIR) $(SERVER_CORE_OBJ)
	ar rcs $@ $(SERVER_CORE_OBJ)

$(SERVER_CORE_OBJ_DIR) :
	mkdir -p $@

$(SERVER_CORE_OBJ_DIR)/%.o : $(SERVER_CORE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean :
	rm -rf $(AUTH_SERVER_OBJ_DIR) $(SERVER_CORE_OBJ_DIR)

fclean : clean
	rm -rf $(AUTH_SERVER_EXE) $(SERVER_CORE_LIB)

re : fclean all

.PHONY : all clean fclean re
