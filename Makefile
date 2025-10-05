CXX = g++
CXXFLAGS = -std=c++17

waf: $(wildcard *.cpp)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

clean:
	rm -f port_scanner
