NAME = test

SRC = main.cpp

OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.cc=.o)

CXX = g++
CXXFLAGS = #-Wall -Werror -Wextra
LDLIBS =  -lhiredis -lpq -lprotobuf -lssl -lcrypto

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

ho :
	echo $(OBJ)
.PHONY : all clean fclean re
