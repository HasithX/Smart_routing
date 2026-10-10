# 🚆 Smart City Multi-Modal Public Transit System (C++17)
> **Topic**: Graph Theory Modeling & Algorithmic Profiling for a Smart City Public Transit System  
> **Language & Standard**: C++17 (Compiled with `g++ -O3 -Wall`)

---

## 📌 Project Overview
This project models an imaginative future smart city where **only public transport (Bus & Train) is permitted**. The transit network is represented as a **Weighted Directed/Undirected Multi-Modal Graph** using an **Adjacency List** ($O(V+E)$ space complexity).

### Key Features
1. **Multi-Modal Route Planning**: Implements a **State-Augmented Dijkstra's Algorithm** with **Transfer Penalties** for mode switching (Train to Bus or vice-versa) using `std::priority_queue` (Min-Heap).
2. **Variable Passenger Demand Simulation**: Simulates dynamic passenger flows across different time windows (Morning Peak, Midday Off-Peak, Evening Peak, Night Low).
3. **Dynamic Congestion Modelling**: Dynamically adjusts road/track transit times based on volume-to-capacity ratios (modified Bureau of Public Roads formula).
4. **Algorithmic Profiling**: Evaluates CPU execution time, throughput (over **100,000 queries/sec**), microsecond latency using `std::chrono::high_resolution_clock`, and space complexity.
5. **Topological Network Visualization**: Generates standalone vector SVG maps (`city_transit_network.svg`) and Graphviz DOT graph definitions.

---

## 🧩 Core Architecture & Module Breakdown

| Module | Purpose | Key Components & Files |
|---|---|---|
| **Core Routing Engine** | Optimal shortest path computation with mode-switching awareness | • State-Augmented Dijkstra implementation (`include/Dijkstra.h`, `src/Dijkstra.cpp`)<br>• Min-Heap priority queue optimization ($O((E \cdot M) \log(V \cdot M))$)<br>• Transfer penalty logic & Route Planner (`include/RoutePlanner.h`, `src/RoutePlanner.cpp`)<br>• Dynamic congestion integration |
| **City Graph & Network Architecture** | Multi-modal network topology modeling | • Node (Location) and Edge data models (`include/Location.h`, `include/Edge.h`)<br>• Adjacency List Multi-Modal Graph data structure (`include/Graph.h`, `src/Graph.cpp`)<br>• CSV dataset ingestion & city topology design (`data/stations.csv`, `data/routes.csv`) |
| **Passenger Demand & Traffic Simulator** | Dynamic city demand & congestion model | • Time-of-day demand matrix modeling (`include/DemandSimulator.h`, `src/DemandSimulator.cpp`)<br>• Peak vs Off-peak passenger trip generation<br>• Urban traffic simulation and edge congestion feedback (BPR function) |
| **Benchmarking, Profiling & Visualization** | Microsecond performance & visual outputs | • Algorithmic profiling with `chrono` (`include/Profiler.h`, `src/Profiler.cpp`)<br>• Scaling benchmarks across $N=100$ to $25,000$ queries<br>• SVG vector transit map generator & Graphviz DOT exporter (`include/Visualizer.h`, `src/Visualizer.cpp`)<br>• Main CLI dashboard (`src/main.cpp`) |

---

## 🏗️ Project Architecture (Modular C++)

```text
Smart_routing/
├── include/
│   ├── Location.h                # Vertex V definition (NodeType, coordinates)
│   ├── Edge.h                    # Edge E definition (Mode, Travel Time, Capacity, BPR congestion)
│   ├── Graph.h                   # MultiModalGraph Adjacency List O(V + E)
│   ├── Dijkstra.h                # State-Augmented Dijkstra with Min-Heap & Transfer Penalty
│   ├── RoutePlanner.h            # High-level journey orchestrator & formatted itinerary
│   ├── DemandSimulator.h         # Peak / Off-peak Passenger Demand Matrix & Traffic Simulator
│   ├── Profiler.h                # High-resolution benchmark suite & throughput scaling
│   └── Visualizer.h              # Standalone SVG vector transit map & DOT exporter
├── src/
│   ├── Location.cpp
│   ├── Edge.cpp
│   ├── Graph.cpp
│   ├── Dijkstra.cpp
│   ├── RoutePlanner.cpp
│   ├── DemandSimulator.cpp
│   ├── Profiler.cpp
│   ├── Visualizer.cpp
│   └── main.cpp                  # Main interactive CLI application
├── data/
│   ├── stations.csv              # Colombo Smart City Transit Stations & Hubs
│   └── routes.csv                # Bus and Train route connections
├── tests/
│   └── test_routing.cpp          # C++ unit tests for routing and graph correctness
├── Makefile                      # Standard C++ build system (make, make run, make test)
└── README.md                     # Documentation & Group contributions
```

---

## 🚀 Getting Started

### 1. Requirements
Ensure `g++` (supporting C++17) and `make` are installed:
```bash
sudo apt install g++ make
```

### 2. Building & Running the Project
```bash
make run
```
This compiles the C++ files and immediately launches the interactive CLI application.

### 3. Running Unit Tests
```bash
make test
```

### 4. Cleaning Build Files
```bash
make clean
```

---

## 🧠 Key Algorithmic & Technical Architecture Decisions

1. **Why Adjacency List ($O(V + E)$) instead of Adjacency Matrix ($O(V^2)$)?**
   - The transit network is sparse ($|E| \ll |V|^2$). For 12 stations, an adjacency matrix allocates 144 cells, whereas the real network has only 32 edges. Adjacency lists minimize RAM and allow iterating over neighbors in $O(\text{deg}(v))$.

2. **Why does Standard Dijkstra fail for Multi-Modal Networks?**
   - Standard Dijkstra stores `(cost, u)`. Arriving at node $u$ by Train vs Bus alters the transition cost to subsequent edges because switching modes incurs a physical **transfer penalty** (walking between platforms, waiting for connecting vehicle). Expanding states to `(u, mode)` guarantees the globally optimal route.

3. **How does C++ `std::priority_queue` achieve $O(\log N)$ performance?**
   - In C++, `std::priority_queue` is backed by a binary heap. Extracting the minimum (`pop`) and inserting states (`push`) take logarithmic time $O(\log(|V| \cdot |M|))$.

4. **Throughput & Efficiency**:
   - The compiled C++ binary executes shortest path calculations in under **10 microseconds** per query, sustaining over **106,000 queries per second (QPS)**.

---

## 🛠️ Git Collaboration & Branching Workflow

This section outlines the standard Git feature branch workflow used to collaborate cleanly without merge conflicts.

### 📌 Development Best Practices:
1. **Never commit directly to `main` without testing!**
2. Always create a personal feature branch for new modules or bug fixes.
3. Run `make clean` before staging/committing to prevent binary/temporary files from entering Git.

---

### 🔄 Standard Workflow (Step-by-Step)

#### Step 1: Update local `main` with latest code
```bash
git checkout main
git pull origin main
```

#### Step 2: Create a feature branch
```bash
# Format: git checkout -b feature/<branch-name>
# Example:
git checkout -b feature/graph-connectivity
```

#### Step 3: Implement your code and test
Edit files in your branch, then test:
```bash
make test
make run
```

#### Step 4: Clean build artifacts before committing
```bash
make clean
```
*(This automatically removes `*.o` object files and temporary test outputs so Git stays 100% clean).*

#### Step 5: Check status, stage, and commit
```bash
# Check modified files
git status

# Stage all project changes
git add .

# Commit with a clear message
git commit -m "feat(module): add BFS connectivity validation and custom routes"
```

#### Step 6: Push your feature branch to GitHub
```bash
git push -u origin feature/<feature-name>
```

#### Step 7: Merge into `main` (Pull Request)
1. Go to GitHub (`https://github.com/HasithX/Smart_routing`).
2. Click **"Compare & pull request"** next to your pushed branch.
3. Review changes and click **"Merge pull request"** -> **"Confirm merge"**.
4. Switch back to `main` and pull the newly merged code:
   ```bash
   git checkout main
   git pull origin main
   ```

