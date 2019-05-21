PROJECTNAME = sigsegv-handler
SRC = handler.hpp
TEST = test
TESTS = nullptr left-out-of-bound right-out-of-bound invalid-permissions
POSTFIX = -test.cpp

all: $(PROJECTNAME)

tests: $(TESTS)

$(PROJECTNAME): $(SRC)
	g++ -std=c++17 -c $(SRC)

$(TESTS): $(PROJECTNAME)
	g++ -std=c++17 -o $@ $(TEST)/$@$(POSTFIX)

clean:
	rm -vf $(PROJECTNAME) *.o $(TESTS)
