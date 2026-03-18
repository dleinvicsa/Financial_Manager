# Переменная для компилятора
CXX = g++
# Флаги компиляции (добавляем отладку и предупреждения)
CXXFLAGS = -Wall -g

# Имя итогового файла
TARGET = finance_manager

# Список всех .cpp файлов (автоматически находит их в src/ и текущей папке)
SRCS = main.cpp src/Category.cpp src/Transaction.cpp src/Wallet.cpp

# Правило по умолчанию
all: $(TARGET)

# Как собрать итоговый файл
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Как очистить проект от скомпилированных файлов
clean:
	rm -f $(TARGET)
