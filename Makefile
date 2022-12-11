GPP_FLAGS = --std=c++20 -Wall -Wextra -Wno-unused-parameter -Wno-reorder -Werror

all: spell_sos spell_sos_release

spell_sos : main.cpp
	g++ -g $(GPP_FLAGS) -O0 $< -o $@
	
spell_sos_release : main.cpp
	g++ $(GPP_FLAGS) -O3 $< -o $@
