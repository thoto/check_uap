LDFLAGS+= -ljsoncpp
CXXFLAGS+= -g -Wall -Wunused -Wextra -pedantic -std=c++11 -DRUN_TEST

check_unifi_state: main.o actions.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

main.o: actions.h main.h
actions.o: actions.h main.h

all: check_unifi_state

clean:
	rm *.o
	test -f check_unifi_state && rm check_unifi_state
