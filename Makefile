INC_DIRS = \
    ./src \
    ./include \
    /usr/include

LIB_DIRS = \
    ./include
    
SRC_DIR = \
    ./src ./src/hud/core ./src/hud/instruments/flight ./src/hud/instruments/navigation ./src/hud/instruments/attitude ./src/gfx ./src/gfx/core ./src/gfx/geometry ./src/gfx/rendering ./src/gfx/terrain ./src/gfx/skybox ./src/flight ./src/flight/dlfdm ./src/flight/data ./src/util ./src/mission ./src/ui/menu ./src/ui/overlay \
    ./src/core ./src/crash ./src/systems ./src/states
    
PROJECT_NAME = FlightSim-HUD
MAIN_CXX = main
GLAD_CXX = glad-460

# Encontrar todos los archivos .cpp conservando su ruta relativa
CPP_SOURCES = $(shell find src -type f -name '*.cpp')
# Convertir rutas de archivos a objetos dentro de build/ conservando carpetas
CPP_OBJECTS = $(patsubst src/%.cpp,$(BUILD_DIR)/src/%.o,$(CPP_SOURCES))

# Lista completa de objetos
OBJ_LIST = $(BUILD_DIR)/$(GLAD_CXX).o $(CPP_OBJECTS)

# Compile with debug symbols
USERCPPFLAGS = -g -Wall -std=c++17

include ./Makefile.master
