LDFLAGS+= -ljsoncpp
CXXFLAGS+= -g -Wall -Wunused -Wextra -pedantic -std=c++11

check_unifi_state: main.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

all: check_unifi_state

clean:
	rm *.o
	test -f check_unifi_state && rm check_unifi_state
