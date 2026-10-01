# 🚆 Smart City Multi-Modal Public Transit System
> **Course**: UCSC IS 2202 / IS 2110 (Advanced Data Structures and Algorithms)  
> **Topic**: Graph Theory Modeling & Algorithmic Profiling for a Smart City Public Transit System

---

## 📌 Project Overview
This project models an imaginative future smart city where **only public transport (Bus & Train) is permitted**. The transit network is represented as a **Weighted Directed/Undirected Multi-Modal Graph** using an **Adjacency List** ($O(V+E)$ space complexity).

### Key Features
1. **Multi-Modal Route Planning**: Implements a **State-Augmented Dijkstra's Algorithm** with **Transfer Penalties** for mode switching (Train to Bus or vice-versa).
2. **Variable Passenger Demand Simulation**: Simulates dynamic passenger flows across different time windows (Morning Peak, Midday Off-Peak, Evening Peak, Night Low).
3. **Dynamic Congestion Modelling**: Dynamically adjusts road/track transit times based on volume-to-capacity ratios (modified Bureau of Public Roads formula).
4. **Algorithmic Profiling**: Evaluates CPU execution time, theoretical vs practical space complexity, bottleneck detection, and `cProfile` analysis.
5. **Topological Network Visualization**: Visualizes the multi-modal network using `networkx` and `matplotlib`.

---

## 👥 Group Members & Work Breakdown (4 Members)

| Member | Primary Role | Core Modules & Responsibilities |
|---|---|---|
| **Member 1** | **Core Routing Engine** | • State-Augmented Dijkstra implementation (`src/algorithms/dijkstra.py`)<br>• Min-Heap priority queue optimization ($O((E \cdot M) \log(V \cdot M))$)<br>• Transfer penalty logic & Route Planner (`src/algorithms/router.py`)<br>• Dynamic congestion integration |
| **Member 2** | **City Graph & Network Architecture** | • Node (Location) and Edge data models (`src/models/location.py`, `src/models/edge.py`)<br>• Adjacency List Multi-Modal Graph data structure (`src/models/graph.py`)<br>• CSV dataset ingestion & city topology design (`data/stations.csv`, `data/routes.csv`) |
| **Member 3** | **Passenger Demand & Traffic Simulator** | • Time-of-day demand matrix modeling (`src/simulation/demand_matrix.py`)<br>• Peak vs Off-peak passenger trip generation<br>• Urban traffic simulation and edge congestion feedback (`src/simulation/simulator.py`) |
| **Member 4** | **Benchmarking, Profiling & Visualization** | • Algorithmic profiling with `cProfile` (`src/profiling/profiler.py`)<br>• Scaling benchmarks across $N=50$ to $2500$ queries<br>• Graph topology rendering & route highlighting (`src/profiling/visualizer.py`)<br>• Main CLI dashboard (`main.py`) |

---

## 🏗️ Project Architecture

```text
Smart_Routing/
├── data/
│   ├── stations.csv              # Stations, Stops & Interchange Hubs
│   └── routes.csv                # Bus and Train route connections
├── src/
│   ├── models/                   # (Member 2) Data Structures & Graph Representation
│   │   ├── location.py           # Vertex V definition (NodeType, coordinates)
│   │   ├── edge.py               # Edge E definition (Mode, Travel Time, Capacity)
│   │   └── graph.py              # MultiModalGraph Adjacency List O(V + E)
│   ├── algorithms/               # (Member 1 - Lead) Core Algorithmic Engine
│   │   ├── dijkstra.py           # State-Augmented Dijkstra with Transfer Penalty
│   │   └── router.py             # RoutePlanner high-level API
│   ├── simulation/               # (Member 3) Dynamic Traffic Simulation
│   │   ├── demand_matrix.py      # Peak / Off-peak Passenger OD Matrix
│   │   └── simulator.py          # Traffic flow and congestion updater
│   └── profiling/                # (Member 4) Performance & Visualization
│       ├── profiler.py           # cProfile & CPU Scaling Benchmarks
│       └── visualizer.py         # NetworkX & Matplotlib transit map
├── tests/
│   ├── test_graph.py             # Graph data structure unit tests
│   └── test_routing.py           # Dijkstra & transfer penalty tests
├── requirements.txt              # Project dependencies
├── Makefile                      # Quick execution commands (make run, make test)
└── main.py                       # Application Entry Point
```

---

## 🚀 Getting Started

### 1. Installation
Ensure Python 3.9+ is installed:
```bash
pip install -r requirements.txt
```

### 2. Running the System
Using the `Makefile`:
```bash
make run
```
Or directly using Python:
```bash
python3 main.py
```

### 3. Running Unit Tests
```bash
make test
```

### 4. Running Algorithmic Profiling
```bash
make profile
```

---

## 🧠 Algorithmic & Data Structure Highlights for the Viva

1. **Why Adjacency List ($O(V + E)$) instead of Adjacency Matrix ($O(V^2)$)?**
   - Public transit networks are **sparse graphs** where degree $d(v) \ll |V|$. An adjacency matrix wastes quadratic space $O(V^2)$ and slows neighbor iterations to $O(V)$. The adjacency list achieves optimal space $O(V+E)$ and neighbor exploration in $O(\text{deg}(v))$.

2. **Why does Standard Dijkstra fail for Multi-Modal Networks?**
   - Standard Dijkstra tracks only `(cost, u)`. When switching transport modes (e.g. Train $\to$ Bus), passengers incur a **transfer penalty** (walking between platforms, waiting for connecting vehicle). Expanding states to `(u, mode)` ensures path optimality without needing artificial graph duplication.

3. **Heap Operations & Complexity**:
   - Uses Python's `heapq` (Binary Min-Heap).
   - Extraction of minimum: $O(\log (|V| \cdot |M|))$.
   - Key relaxations / insertions: at most $|E| \cdot |M|$.
   - Total time: $O((|E| \cdot |M|) \log (|V| \cdot |M|)) \approx O(E \log V)$.

---

## 🛠️ Git Collaboration Workflow

### For Member 1 (Project Setup & Lead):
```bash
# 1. Create and switch to your feature branch(add your name)
git checkout -b feature/setup-and-core-architecture

# 2. Stage and commit your changes
git add .
git commit -m "feat: complete project architecture, multi-modal Dijkstra, and simulation"

# 3. Push your branch to GitHub
git push -u origin feature/setup-and-core-architecture

# 4. Merge to main (locally or via GitHub Pull Request)
git checkout main
git merge feature/setup-and-core-architecture
git push origin main
```

### For Members 2, 3, and 4 (Pulling and branching):
```bash
# 1. Pull the updated main branch
git checkout main
git pull origin main

# 2. Create their respective branch to work on improvements (add your name)
git checkout -b feature/<member-name>-<component>
```
