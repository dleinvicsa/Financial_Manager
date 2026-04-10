CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

TARGET = finance_manager

SRCS = main.cpp \
	src/AppEngine.cpp \
	src/Category.cpp \
	src/DataManager.cpp \
	src/Statistics.cpp \
	src/Transaction.cpp \
	src/Utils.cpp \
	src/Wallet.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) .DS_Store
	rm -rf $(TARGET).dSYM

run-web:
	node web/server.js
