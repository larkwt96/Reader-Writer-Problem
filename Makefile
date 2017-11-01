CXXFLAGS = -Wall -pthread #-std=c11
EXTRAFLAGS = -Wextra -g
SOURCES = soln.c
EXE = soln
TEST = dsoln
PKG_NAME = Wilson_Lucas-Reader_Writer_Soln.tar
TEST_FILE = test_file

# Other variables, don't change
PKG_FILES = $(SOURCES) Makefile README

# Default
all: $(EXE)

# Executable
$(EXE): $(SOURCES) $(TEST_FILE)
	@echo "Creating $(EXE)"
	gcc $(CXXFLAGS) -o $(EXE) $(SOURCES)

$(TEST_FILE):
	echo -n > $(TEST_FILE)

# Test
test: CXXFLAGS += $(EXTRAFLAGS)
test: $(TEST)
$(TEST):
	@echo "Creating $(TEST)"
	gcc $(CXXFLAGS) -o $(TEST) $(SOURCES)

# Package
.PHONY: pkg
pkg: $(PKG_NAME)
$(PKG_NAME): $(PKG_FILES)
	-rm -f $(PKG_NAME)
	@echo "Creating $(PKG_NAME)"
	tar cf $(PKG_NAME) $(PKG_FILES)

# Clean
.PHONY: clean
clean:
	@echo "Cleaning"
	rm -rf *.o $(TEST) $(EXE) $(PKG_NAME) $(TEST).dSYM $(EXE).dSYM $(TEST_FILE)
