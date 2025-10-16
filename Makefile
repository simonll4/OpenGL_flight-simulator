INC_DIRS = \
    ./src \
    ./include \
    /usr/include

LIB_DIRS = \
    ./include
    
SRC_DIR = \
    ./src ./src/hud ./src/gfx ./src/flight ./src/util
    
PROJECT_NAME = Skybox-Demo
MAIN_CXX = main
GLAD_CXX = glad-460

# Encontrar todos los archivos .cpp en src/ y subdirectorios
CPP_SOURCES = $(wildcard src/*.cpp) $(wildcard src/hud/*.cpp) $(wildcard src/gfx/*.cpp) $(wildcard src/flight/*.cpp) $(wildcard src/util/*.cpp)
# Convertir rutas de archivos a nombres de objetos (sin subdirectorios)
CPP_OBJECTS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(CPP_SOURCES)))

# Lista completa de objetos
OBJ_LIST = $(BUILD_DIR)/$(GLAD_CXX).o $(CPP_OBJECTS)

# Compile with debug symbols
USERCPPFLAGS = -g -Wall -std=c++17

include ./Makefile.master
