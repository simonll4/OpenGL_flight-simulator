INC_DIRS = \
    ./src \
    ./include \
    /usr/include

LIB_DIRS = \
    ./include
    
SRC_DIR = \
    ./src ./src/hud/core ./src/hud/instruments/flight ./src/hud/instruments/navigation ./src/gfx ./src/gfx/core ./src/gfx/geometry ./src/gfx/rendering ./src/gfx/terrain ./src/gfx/skybox ./src/flight ./src/util
    
PROJECT_NAME = FlightSim-HUD
MAIN_CXX = main
GLAD_CXX = glad-460

# Encontrar todos los archivos .cpp en src/ y subdirectorios (nueva estructura HUD organizada)
CPP_SOURCES = $(wildcard src/*.cpp) $(wildcard src/hud/core/*.cpp) $(wildcard src/hud/instruments/flight/*.cpp) $(wildcard src/hud/instruments/navigation/*.cpp) $(wildcard src/gfx/*.cpp) $(wildcard src/gfx/core/*.cpp) $(wildcard src/gfx/geometry/*.cpp) $(wildcard src/gfx/rendering/*.cpp) $(wildcard src/gfx/terrain/*.cpp) $(wildcard src/gfx/skybox/*.cpp) $(wildcard src/flight/*.cpp) $(wildcard src/util/*.cpp)
# Convertir rutas de archivos a nombres de objetos (sin subdirectorios)
CPP_OBJECTS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(CPP_SOURCES)))

# Lista completa de objetos
OBJ_LIST = $(BUILD_DIR)/$(GLAD_CXX).o $(CPP_OBJECTS)

# Compile with debug symbols
USERCPPFLAGS = -g -Wall -std=c++17

include ./Makefile.master
