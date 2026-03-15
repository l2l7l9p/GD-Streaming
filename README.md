# GD-Streaming

This is the streaming algorithm for approximating the $k$-graphlet (i.e., connected induced subgraphs of $k$ vertices) distribution as well as sampling $k$-graphlets uniformly. Specifically, the program is given a simple undirected graph of $n$ vertices where the edge list can only be accessed by streaming, a small constant $k$, and necessary parameters. It can:

- sample $k$-graphlets uniformly;

- approximate the $k$-graphlet distribution of the graph.

# Reference

Please cite our paper if you use this project.

```
Marco Bressan, T-H. Hubert Chan, Qipeng Kuang, Mauro Sozio. An Efficient Streaming Algorithm for Approximating Graphlet Distributions. In SIGMOD 2026.
```

# Main Code

The algorithm is implemented in C++ in the directory `src`.

## Format of the Edge List File

Suppose the graph has $n$ vertices and $m$ edges: $(x_1, y_1), \cdots, (x_m, y_m)$. The graph should be undirected and not contain self-loops or duplicated edges. The vertices are labeled from $1$ to $n$.

Two formats are supported:

- Plain Text: The edge list file should be in the following format (a header followed by all edges).

```
u,v
x1,y1
x2,y2
x3,y3
...
xm,ym
```

- Binary (Recommended): The edge list file is binary and in the following format. Each number uses 32 bits.

```
x1 y1 x2 y2 x3 y3 ... xm ym
```

## Pre-Requests

Prepare the edge list file and the configuration file (e.g., `config.yml`), and put them in the same directory with the code.

The parameters in `config.yml` are:

- `func`: the function you want to run, including:
  
  - `ugs`: uniform graphlet sampling
  
  - `gd-rejection`: approximate the graphlet distribution via GDRejection
  
  - `gd-counter`: approximate the graphlet distribution via GDCounter

- `n`: the number of vertices,

- `edgelistfile`: the name of the edge list file,

- `edgelistmode`: the mode of storing the edge list which has four choices:
  
  - `local-plaintxt`: The program reads the edges from the local file in plain text format regarding it as a streaming source. Everything except the edge list is stored in the main memory.
  
  - `local-binary`: The program reads the edges from the local file in binary format regarding it as a streaming source. Everything except the edge list is stored in the main memory.

  - `memory`: (TODO) The program stores everything (including the edge list) in main memory.
  
- `ddordermode`: choose an of the following algorithms to compute the DD order,
  - `approx`
  
  - `approx-heuristic`
  
  - `ES`
  
  - `ES-heuristic`
  
- (optional) `es_c`: the constant $c$ for the faster algorithm, (necessary if `ES` or `ES-heuristic` is selected, otherwise no need)

- `epsilon`: the parameter for DD-order,

- `k`: the parameter for graphlet sampling,

- `target`: we do sampling in batches until there are `target` successful trials.

- `MAX_EDGES`: the maximum number of edges we store during the computation of DD-order and sampling. We will perform sampling with a batch size `MAX_EDGES/(k*k)`.

- (optional) `ddorderfile`: a file containing a list representing the DD-order, so that we do not compute it again.

## Usage

GNU G++ is the recommended compiler for the code. Simply use `make` to compile the code:

```
make
```

or compile it by the following command if you don't have `make` installed:

```
g++ main.cpp edge.cpp graph.cpp utils.cpp -o main -std=c++17 -O3
```

Then, run it with the argument specifying the config file, e.g. :

```
./main config.yml
```

It will print the logs on the terminal and produce the following three files:

- `logs.log`: the logs.
- `DD_order.bin`: the DD-order in binary format which contains `n` numbers, each using 32 bits. (This file is produced only when `ddorderfile` is NOT specified in the config.)
- `samples.txt` or `graphlet_distribution.txt`:
  - If `func` is `ugs`, it produces `samples.txt` in plain text. The file has only one line, containing `target*k+2` space-separated integers. The first integer is the number of samples. The second integer is `k`. Then every `k` integers indicate a sample (i.e., the vertex IDs of the graphlet).
  - If `func` is `gd-rejection` or `gd-counter`, it produces `graphlet_distribution.txt`. The adjacency mask for a connected graph of $k$ vertices is a $\frac{k(k-1)}{2}$-bit integer indicating whether edge$(1,2)$, ..., edge$(1,k)$, edge$(2,3)$, ..., edge$(2,k)$, ..., edge$(k-1,k)$ exist respectively. The adjacency mask for an isomorphic class is the minimum adjacency mask among the graphs in the class. For each isomorphic class of connected graphs of $k$ vertices, the file prints a line consisting of the mask of the class, and its graphlet distribution in the input graph.


Note:

- The processing of the yaml file in this code is somehow stupid so please follow the required yaml format strictly.

# Generalization

- You can modify `edge.h` and `edge.cpp` where the access to the edge list is defined. In this way, you can make it a true streaming algorithm. For example, you can access the edge list from the Internet.

- The usage of the preprocessing and the sampling functions are shown in `main.cpp`. You can treat this project as a library for your own project.

# Other Tools

Useful tools are provided in the directory `tools`.

## Generate Random Graphs

`gen_edge.cpp` is to generate the edge list of a random undirected graph. It takes 2 arguments `n` and `p`, indicating the number of vertices and the probability of each edge.

G++ is the recommended compiler for this code.

```
g++ gen_edge.cpp -o gen_edge
```

It produces a file `edge_list.csv` in the same directory. Then, for example, run

```
./gen_edge 10 0.5
```

## Convert Edge List Format

`convert_plaintxt_binary.cpp` is to convert the format of the edge list file from plain text to binary, or from binary to plain text.

Compile the code by

```
g++ convert_plaintxt_binary.cpp -o convert_plaintxt_binary -O3
```

To convert plain text to binary, run

```
./convert_plaintxt_binary pb <PLAIN_TEXT_FILE> <BINARY_FILE>
```

To convert binary to plain text, run

```
./convert_plaintxt_binary bp <BINARY_FILE> <PLAIN_TEXT_FILE>
```

## Reformat Dataset from Konect

`konect_reformat.py` is to reformat the dataset from http://konect.cc/networks/ to binary format required above, removing duplicated edges and self-loops.

The usage is

```
python reformat.py <INPUT_FILE> <OUTPUT_FILE>
```

For example,

```
python reformat.py out.edit-biwikibooks edge_list.bin
```

## Print the $k$-Graphlet Distribution

`probability.cpp` to print the $k$-graphlet distribution of the sample result so that you can check its uniformity.

Compile the code by

```
g++ probability.cpp -o probability -std=c++17
```

It takes one argument as the text file containing the sampling result (for example, `samples.txt`). Run it by

```
./probability samples.txt
```

It will produce a file `probability.txt` showing the probability of each $k$-graphlet in `samples.txt`.

## Memory Monitor

`memory_monitor.sh` is to monitor the memory usage of a program. Run the script by the following command:

```
sh memory_monitor.sh '<COMMAND>' <ID_STRING>
```

`COMMAND` is the task we need to run, and `ID_STRING` is to identify the task. It runs `COMMAND` for 5 times. In each run, it records the virtual memory of the task every 2 seconds by looking at `ps aux` and prints the records to `memory_<ID_STRING>_1.log`, ..., `memory_<ID_STRING>_5.log`.
