# Makefile for TCP Server

# Adjust these paths based on your Qt installation
QTDIR = C:\Qt\Qt6.6.3
QMAKE = $(QTDIR)/bin/qmake
CXX = g++
CXXFLAGS = -Wall -std=c++11

SERVER_PRO = Server.pro
SERVER_TARGET = server

all: $(SERVER_TARGET)

$(SERVER_TARGET):
	$(QMAKE)	$(SERVER_PRO)
	$(MAKE)	-C	$(dir	$(SERVER_PRO))
	$(MAKE)	-C	$(dir	$(SERVER_PRO))	clean
	@echo	"Build complete. Run './$(SERVER_TARGET)' to start the server."

clean:
	$(MAKE)	-C	$(dir	$(SERVER_PRO))	clean