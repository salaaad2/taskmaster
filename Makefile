#==============================================================================#
#    TASKMASTER        (  //
#    Makefile          ( )/
#    by salade         )(/
#   ________________  ( /)
#  ()__)____________)))))   :^}
#==============================================================================#
# taskmaster c++ makefile
# @version 1.0
#
default: all

#==============================================================================#
#--------------------------------- SHELL --------------------------------------#
#==============================================================================#
SHELL			:= /bin/sh
OS				 = $(shell uname)

#==============================================================================#
#------------------------------ DIRECTORIES -----------------------------------#
#==============================================================================#
SRCS_DIR		 = src/
OBJS_DIR		 = obj/

#==============================================================================#
#--------------------------------- FILES --------------------------------------#
#==============================================================================#
SRCS_NAME		 = main
SRCS_NAME		 += Process
SRCS_NAME		 += Supervisor
SRCS_NAME		 += StringUtils
SRCS_NAME		 += Utils
#------------------------------------------------------------------------------#
INCS_NAME		 = main
INCS_NAME		 += Process
INCS_NAME		 += Supervisor
INCS_NAME		 += StringUtils
INCS_NAME		 += Utils
SRCS			 = $(addprefix ${SRCS_DIR}, $(addsuffix .cpp, ${SRCS_NAME}))
#------------------------------------------------------------------------------#
#------------------------------------------------------------------------------#
INCS			 = $(addprefix ${SRCS_DIR}, $(addsuffix .hpp, ${INCS_NAME}))
INCS			+= $(patsubst %.cpp,%.hpp,${SRCS})
#------------------------------------------------------------------------------#
OBJS			 = $(patsubst ${SRCS_DIR}%.cpp,${OBJS_DIR}%.o,${SRCS})
#------------------------------------------------------------------------------#
NAME			 = taskmaster
#------------------------------------------------------------------------------#

#==============================================================================#
#-------------------------------- COMPILER ------------------------------------#
#==============================================================================#
CC				 = clang++
#------------------------------------------------------------------------------#
CFLAGS			 = -std=c++17
CFLAGS			+= -Wall
CFLAGS			+= -Wextra
CFLAGS			+= -Werror
CFLAGS			+= -pedantic
#------------------------------------------------------------------------------#
LDFLAGS			 = -lyaml-cpp -L./ext/yaml-cpp/build

#==============================================================================#
#--------------------------------- UNIX ---------------------------------------#
#==============================================================================#
RM				= rm -rf
MKDIR			= mkdir -p

#==============================================================================#
#--------------------------------- RULES --------------------------------------#
#==============================================================================#
#------------------------------------------------------------------------------#
${OBJS_DIR}%.o:	${SRCS_DIR}%.cpp ${INCS}
	${CC} -c ${CFLAGS} ${CDEFS} -I ext/yaml-cpp/include/ -o $@ $<
#------------------------------------------------------------------------------#
${OBJS_DIR}:
	${MKDIR} ${OBJS_DIR}
	cmake --build ./ext/yaml-cpp/build
#------------------------------------------------------------------------------#
$(NAME): ${OBJS}
	${CC} ${CFLAGS} ${CDEFS} -o ${NAME} ${OBJS} ${LDFLAGS}
#------------------------------------------------------------------------------#
all: ${OBJS_DIR} ${NAME}
#------------------------------------------------------------------------------#
debug: CFLAGS += -g3
debug: all
#------------------------------------------------------------------------------#
asan: CFLAGS += -g3
asan: CFLAGS += -fsanitize=address
asan: all
#------------------------------------------------------------------------------#
msan: CFLAGS += -g3
msan: CFLAGS += -fsanitize=memory
msan: CFLAGS += -fsanitize-memory-track-origins
msan: CFLAGS += -fno-common
msan: CFLAGS += -fno-omit-frame-pointer
msan: all
#------------------------------------------------------------------------------#
clean:
	${RM} ${OBJS_DIR} vgcore*
#------------------------------------------------------------------------------#
fclean: clean
	${RM} ${NAME} ${NAME}.core ${NAME}.dSYM/ libyaml-cpp.a
#------------------------------------------------------------------------------#
re: fclean all
#------------------------------------------------------------------------------#
run: all
#------------------------------------------------------------------------------#
.PHONY:	all clean clean fclean re debug asan run
