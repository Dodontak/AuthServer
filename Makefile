#======= AuthServer ========#
AUTH_SERVER_DIR = AuthServer
AUTH_SERVER_EXE = test
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
CLIENT_DIR = DummyClient
CLIENT_EXE = client
CLIENT_SRC_FILE = DummyClient.cpp \
	Protocol.pb.cc \
	ServerPacketHandler.cpp
CLIENT_SRC = $(addprefix $(CLIENT_DIR)/, $(CLIENT_SRC_FILE))
CLIENT_OBJ_DIR = $(CLIENT_DIR)/ObjectFiles
CLIENT_OBJ_FILE = $(CLIENT_SRC_FILE:.cpp=.o)
CLIENT_OBJ_FILE := $(CLIENT_OBJ_FILE:.cc=.o)
CLIENT_OBJ = $(addprefix $(CLIENT_OBJ_DIR)/, $(CLIENT_OBJ_FILE))
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

all : auth cli

#======= AuthServer ========#
auth : $(AUTH_SERVER_EXE)

$(AUTH_SERVER_EXE) : $(AUTH_SERVER_OBJ_DIR) $(SERVER_CORE_LIB) $(AUTH_SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $(AUTH_SERVER_OBJ) $(SERVER_CORE_LIB) $(LDLIBS) -o $(AUTH_SERVER_EXE)

$(AUTH_SERVER_OBJ_DIR) :
	mkdir -p $@

$(AUTH_SERVER_OBJ_DIR)/%.o : $(AUTH_SERVER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(SERVER_CORE_INC) -o $@ -c $<

$(AUTH_SERVER_OBJ_DIR)/%.o : $(AUTH_SERVER_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(SERVER_CORE_INC) -o $@ -c $<
#======= AuthServer ========#


#======= DummyCient ========#
cli : $(CLIENT_EXE)

$(CLIENT_EXE) : $(CLIENT_OBJ_DIR) $(CLIENT_OBJ) $(SERVER_CORE_LIB)
	$(CXX) $(CXXFLAGS) $(CLIENT_OBJ) $(SERVER_CORE_LIB) $(LDLIBS) -o $(CLIENT_EXE)

$(CLIENT_OBJ_DIR) :
	mkdir -p $@

$(CLIENT_OBJ_DIR)/%.o : $(CLIENT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(SERVER_CORE_INC) -o $@ -c $<

$(CLIENT_OBJ_DIR)/%.o : $(CLIENT_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(SERVER_CORE_INC) -o $@ -c $<
#======= DummyCient ========#

#======= ServerCore ========#
$(SERVER_CORE_LIB) : $(SERVER_CORE_OBJ_DIR) $(SERVER_CORE_OBJ)
	ar rcs $@ $(SERVER_CORE_OBJ)

$(SERVER_CORE_OBJ_DIR) :
	mkdir -p $@

$(SERVER_CORE_OBJ_DIR)/%.o : $(SERVER_CORE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(AUTH_SERVER_INC) -o $@ -c $<
#======= ServerCore ========#

clean :
	rm -rf $(AUTH_SERVER_OBJ_DIR) $(CLIENT_OBJ_DIR) $(SERVER_CORE_OBJ_DIR)

fclean : clean
	rm -rf $(AUTH_SERVER_EXE) $(CLIENT_EXE) $(SERVER_CORE_LIB)

re : fclean all

.PHONY : all clean fclean re
