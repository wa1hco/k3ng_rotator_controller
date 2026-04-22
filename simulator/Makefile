CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -I../k3ng_rotator_controller

SIM_SOURCES := pc_rotator_sim.cpp \
	../k3ng_rotator_controller/rotator_simulator.cpp \
	../k3ng_rotator_controller/rotator_protocol_shim.cpp
SIM_BINARY := pc_rotator_sim

GUI_SOURCES := gui_server.cpp
GUI_BINARY := gui_server

.PHONY: build run gui gui-run clean

build: $(SIM_BINARY) $(GUI_BINARY)

run: $(SIM_BINARY)
	./$(SIM_BINARY)

gui: $(GUI_BINARY)

gui-run: $(GUI_BINARY)
	./$(GUI_BINARY)

$(SIM_BINARY): $(SIM_SOURCES)
	$(CXX) $(CXXFLAGS) $(SIM_SOURCES) -o $(SIM_BINARY)

$(GUI_BINARY): $(GUI_SOURCES)
	$(CXX) $(CXXFLAGS) $(GUI_SOURCES) -o $(GUI_BINARY)

clean:
	rm -f $(SIM_BINARY) $(GUI_BINARY)
