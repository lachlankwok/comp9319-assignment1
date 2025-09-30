CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -O2

# Targets to build both programs
all: lencode ldecode

# Build lencode from lencode.cpp
lencode: lencode.cpp
	$(CXX) $(CXXFLAGS) -o lencode lencode.cpp

# Build ldecode from ldecode.cpp
ldecode: ldecode.cpp
	$(CXX) $(CXXFLAGS) -o ldecode ldecode.cpp

# Run encode with a file: make run-encode input=file.txt
run-lencode: lencode
	./lencode $(input)

# Run decode with a file: make run-decode input=file.lzw
run-ldecode: ldecode
	./ldecode $(input)

# Remove built files
clean:
	rm -f lencode ldecode
