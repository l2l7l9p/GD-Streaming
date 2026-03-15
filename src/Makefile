main: $(mainfiles)
	g++ main.cpp $(components) -o main -std=c++17 -O3
debug: $(mainfiles)
	g++ main.cpp $(components) -o main -std=c++17 -g

experiment: $(experimentfiles)
	g++ experiment.cpp $(components) -o experiment -std=c++17 -O3
experimentdebug: $(experimentfiles)
	g++ experiment.cpp $(components) -o experiment -std=c++17 -g

components = graph.cpp edge.cpp all_graphlets.cpp config.cpp $(utils_components)
heads = graph.h edge.h all_graphlets.h config.h $(utils_heads)
utils_components = utils/logging.cpp utils/ds.cpp
utils_heads = utils/logging.h utils/randomness.h utils/ds.h
mainfiles = main.cpp $(components) $(heads)
experimentfiles = experiment.cpp $(components) $(heads)

clean:
	rm -f main experiment