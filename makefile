ifeq ($(wildcard $(UPCXX_INSTALL)/bin/upcxx-meta),)
$(error Please set UPCXX_INSTALL=/path/to/upcxx/install)
endif

NAME=main

# Source files
SRC=main.cpp

# Header files
_INC=

# Libraries
_LIB=eigen/Eigen upcxx

INC=$(patsubst %, -I%, $(_LIB))
INC+=$(patsubst %, -I%, $(_INC))

# Compiler spesifics
CC= $(UPCXX_INSTALL)/bin/upcxx
RUN=$(UPCXX_INSTALL)/bin/upcxx-run
UPCXX_OPT = -g

UPCXX_THREADMODE ?= seq
ENV = env UPCXX_THREADMODE=$(UPCXX_THREADMODE)

NP ?= 4

PTHREAD_FLAGS = -pthread
OPENMP_FLAGS = -fopenmp

$(NAME): $(SRC)
	$(CC) -o $@ $^ $(UPCXX_OPT) $(INC)

.PHONY: run
run: $(NAME)
	$(RUN) -np $(NP) ./$(NAME)

.PHONY: clean
clean:
	rm -f $(NAME)