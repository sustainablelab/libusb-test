SRC := src/main.cpp
EXE := build/$(basename $(notdir $(SRC)))
HEADER_LIST := build/$(basename $(notdir $(SRC))).d

CXXFLAGS_BASE := -std=c++20 -Wall -Wextra
CXXFLAGS_USB := `pkg-config --cflags libusb-1.0`
CXXFLAGS := $(CXXFLAGS_BASE) $(CXXFLAGS_USB)
LDLIBS := `pkg-config --libs libusb-1.0`

default-target: $(EXE)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

.PHONY: $(HEADER_LIST)
$(HEADER_LIST): $(SRC)
	$(CXX) $(CXXFLAGS) -M $^ -MF $@

build/ctags-dlist: ctags-dlist.cpp
	$(CXX) $(CXXFLAGS_BASE) $^ -o $@

.PHONY: tags
tags: $(HEADER_LIST) build/ctags-dlist
	build/ctags-dlist $(HEADER_LIST)
	ctags --c-kinds=+p+x -L headers.txt
	ctags --c-kinds=+p+x -a $(SRC)

.PHONY: what
what:
	@echo
	@echo --- My make variables ---
	@echo
	@echo "CXX              : "$(CXX)
	@echo "CXXFLAGS         : "$(CXXFLAGS)
	@echo "SRC              : "$(SRC)
	@echo "EXE              : "$(EXE)
	@echo "HEADER_LIST      : "$(HEADER_LIST)

.PHONY: how
how:
	@echo
	@echo --- Build and Run ---
	@echo
	@echo "             Vim shortcut    Vim command line (approximate)"
	@echo "             ------------    ------------------------------"
	@echo "Build        ;<Space>        :make -B  <if(error)> :copen"
	@echo "Run          ;r<Space>       :!./build/main"
	@echo "Make tags    ;t<Space>       :make tags"
