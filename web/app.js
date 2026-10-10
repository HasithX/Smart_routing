/**
 * Smart City Public Transit Routing & Profiler Engine
 * Advanced Multi-Modal Transit Data Structures & Algorithms
 * Pure Vanilla JavaScript Client Engine
 */

// --- 1. TRANSIT DATA SPECIFICATION ---
const RAW_STATIONS = [
    { id: "ST01", name: "Kottawa Makumbura MMC", type: "INTERCHANGE_HUB", zone: "Residential", x: 85.0, y: 20.0 },
    { id: "ST02", name: "Maharagama Clock Tower", type: "BUS_STOP", zone: "Commercial", x: 70.0, y: 30.0 },
    { id: "ST03", name: "Nugegoda Supermarket", type: "BUS_STOP", zone: "Commercial", x: 55.0, y: 40.0 },
    { id: "ST04", name: "Maradana Interchange", type: "INTERCHANGE_HUB", zone: "Mixed", x: 55.0, y: 80.0 },
    { id: "ST05", name: "Colombo Fort Station", type: "INTERCHANGE_HUB", zone: "Commercial", x: 45.0, y: 85.0 },
    { id: "ST06", name: "Pettah Central Terminal", type: "BUS_STOP", zone: "Commercial", x: 50.0, y: 85.0 },
    { id: "ST07", name: "Battaramulla Sethsiripaya", type: "BUS_STOP", zone: "Administrative", x: 75.0, y: 70.0 },
    { id: "ST08", name: "Malabe IT Park", type: "BUS_STOP", zone: "Industrial", x: 90.0, y: 70.0 },
    { id: "ST09", name: "University Main Gate", type: "BUS_STOP", zone: "Educational", x: 40.0, y: 50.0 },
    { id: "ST10", name: "Town Hall Hospital", type: "BUS_STOP", zone: "PublicService", x: 50.0, y: 65.0 },
    { id: "ST11", name: "Colombo Harbor Port", type: "TRAIN_STATION", zone: "Industrial", x: 45.0, y: 95.0 },
    { id: "ST12", name: "Bambalapitiya Station", type: "INTERCHANGE_HUB", zone: "Residential", x: 35.0, y: 45.0 },
    { id: "ST13", name: "Wellawatte Station", type: "TRAIN_STATION", zone: "Residential", x: 30.0, y: 38.0 },
    { id: "ST14", name: "Kelaniya Station", type: "TRAIN_STATION", zone: "Residential", x: 65.0, y: 96.0 }
];

const RAW_ROUTES = [
    { route_id: "R_TR01", source: "ST01", target: "ST04", mode: "TRAIN", distance_km: 18.0, base_time_min: 22.0, capacity: 500, bidirectional: true },
    { route_id: "R_TR02", source: "ST04", target: "ST05", mode: "TRAIN", distance_km: 2.5, base_time_min: 4.0, capacity: 600, bidirectional: true },
    { route_id: "R_TR03", source: "ST05", target: "ST11", mode: "TRAIN", distance_km: 3.5, base_time_min: 6.0, capacity: 350, bidirectional: true },
    { route_id: "R_TR04", source: "ST05", target: "ST12", mode: "TRAIN", distance_km: 5.0, base_time_min: 7.0, capacity: 550, bidirectional: true },
    { route_id: "R_BUS01", source: "ST01", target: "ST02", mode: "BUS", distance_km: 3.5, base_time_min: 8.0, capacity: 80, bidirectional: true },
    { route_id: "R_BUS02", source: "ST02", target: "ST03", mode: "BUS", distance_km: 4.5, base_time_min: 12.0, capacity: 80, bidirectional: true },
    { route_id: "R_BUS03", source: "ST03", target: "ST10", mode: "BUS", distance_km: 6.0, base_time_min: 15.0, capacity: 80, bidirectional: true },
    { route_id: "R_BUS04", source: "ST10", target: "ST06", mode: "BUS", distance_km: 3.0, base_time_min: 8.0, capacity: 85, bidirectional: true },
    { route_id: "R_BUS05", source: "ST05", target: "ST06", mode: "BUS", distance_km: 1.0, base_time_min: 3.0, capacity: 90, bidirectional: true },
    { route_id: "R_BUS06", source: "ST06", target: "ST04", mode: "BUS", distance_km: 1.8, base_time_min: 5.0, capacity: 85, bidirectional: true },
    { route_id: "R_BUS07", source: "ST04", target: "ST07", mode: "BUS", distance_km: 8.5, base_time_min: 20.0, capacity: 80, bidirectional: true },
    { route_id: "R_BUS08", source: "ST07", target: "ST08", mode: "BUS", distance_km: 4.0, base_time_min: 10.0, capacity: 80, bidirectional: true },
    { route_id: "R_BUS09", source: "ST12", target: "ST09", mode: "BUS", distance_km: 1.5, base_time_min: 4.0, capacity: 75, bidirectional: true },
    { route_id: "R_BUS10", source: "ST09", target: "ST10", mode: "BUS", distance_km: 2.5, base_time_min: 7.0, capacity: 75, bidirectional: true },
    { route_id: "R_BUS11", source: "ST03", target: "ST09", mode: "BUS", distance_km: 5.0, base_time_min: 14.0, capacity: 80, bidirectional: true },
    { route_id: "R_BUS12", source: "ST10", target: "ST04", mode: "BUS", distance_km: 2.8, base_time_min: 7.0, capacity: 80, bidirectional: true },
    { route_id: "R_TR05", source: "ST12", target: "ST13", mode: "TRAIN", distance_km: 2.0, base_time_min: 4.0, capacity: 400, bidirectional: true },
    { route_id: "R_TR06", source: "ST04", target: "ST14", mode: "TRAIN", distance_km: 8.0, base_time_min: 15.0, capacity: 400, bidirectional: true }
];

// --- 2. PRIORITY QUEUE (MIN-HEAP) ---
class MinPriorityQueue {
    constructor() {
        this.heap = [];
    }

    push(item) {
        this.heap.push(item);
        this._bubbleUp(this.heap.length - 1);
    }

    pop() {
        if (this.isEmpty()) return null;
        const top = this.heap[0];
        const bottom = this.heap.pop();
        if (this.heap.length > 0) {
            this.heap[0] = bottom;
            this._bubbleDown(0);
        }
        return top;
    }

    isEmpty() {
        return this.heap.length === 0;
    }

    _bubbleUp(index) {
        while (index > 0) {
            const parentIndex = Math.floor((index - 1) / 2);
            if (this.heap[index].cost < this.heap[parentIndex].cost) {
                [this.heap[index], this.heap[parentIndex]] = [this.heap[parentIndex], this.heap[index]];
                index = parentIndex;
            } else {
                break;
            }
        }
    }

    _bubbleDown(index) {
        const length = this.heap.length;
        while (true) {
            let leftChild = 2 * index + 1;
            let rightChild = 2 * index + 2;
            let smallest = index;

            if (leftChild < length && this.heap[leftChild].cost < this.heap[smallest].cost) {
                smallest = leftChild;
            }
            if (rightChild < length && this.heap[rightChild].cost < this.heap[smallest].cost) {
                smallest = rightChild;
            }
            if (smallest !== index) {
                [this.heap[index], this.heap[smallest]] = [this.heap[smallest], this.heap[index]];
                index = smallest;
            } else {
                break;
            }
        }
    }
}

// --- 3. GRAPH & BPR MODEL ---
class TransitEdge {
    constructor(routeId, source, target, mode, distanceKm, baseTimeMin, capacity) {
        this.routeId = routeId;
        this.source = source;
        this.target = target;
        this.mode = mode; // "BUS" or "TRAIN"
        this.distanceKm = distanceKm;
        this.baseTimeMin = baseTimeMin;
        this.capacity = capacity;
        this.currentFlow = 0;
    }

    getEffectiveTravelTime() {
        if (this.capacity <= 0) return this.baseTimeMin;
        const vc = this.currentFlow / this.capacity;
        const alpha = (this.mode === "TRAIN") ? 0.15 : 0.60;
        const beta = (this.mode === "TRAIN") ? 2.0 : 2.5;
        const multiplier = 1.0 + alpha * Math.pow(vc, beta);
        return Math.round(this.baseTimeMin * multiplier * 10) / 10;
    }

    addFlow(pax = 1) {
        this.currentFlow += pax;
    }

    resetFlow() {
        this.currentFlow = 0;
    }
}

class TransitGraph {
    constructor() {
        this.nodes = new Map();
        this.adjList = new Map();
        this.init();
    }

    init() {
        this.nodes.clear();
        this.adjList.clear();

        for (const st of RAW_STATIONS) {
            this.nodes.set(st.id, { ...st });
            this.adjList.set(st.id, []);
        }

        for (const rt of RAW_ROUTES) {
            const edge = new TransitEdge(rt.route_id, rt.source, rt.target, rt.mode, rt.distance_km, rt.base_time_min, rt.capacity);
            this.adjList.get(rt.source).push(edge);

            if (rt.bidirectional) {
                const revEdge = new TransitEdge(rt.route_id + "_REV", rt.target, rt.source, rt.mode, rt.distance_km, rt.base_time_min, rt.capacity);
                this.adjList.get(rt.target).push(revEdge);
            }
        }
    }

    resetAllCongestion() {
        for (const edges of this.adjList.values()) {
            for (const edge of edges) {
                edge.resetFlow();
            }
        }
    }

    getNode(id) {
        return this.nodes.get(id);
    }

    getNeighbors(id) {
        return this.adjList.get(id) || [];
    }

    checkConnectivity() {
        if (this.nodes.size === 0) return false;
        const startId = this.nodes.keys().next().value;
        const visited = new Set([startId]);
        const queue = [startId];

        while (queue.length > 0) {
            const curr = queue.shift();
            for (const edge of this.getNeighbors(curr)) {
                if (!visited.has(edge.target)) {
                    visited.add(edge.target);
                    queue.push(edge.target);
                }
            }
        }
        return visited.size === this.nodes.size;
    }
}

// --- 4. STATE-AUGMENTED DIJKSTRA ROUTER ---
class MultiModalDijkstra {
    constructor(graph, defaultTransferPenalty = 6.0) {
        this.graph = graph;
        this.defaultTransferPenalty = defaultTransferPenalty;
    }

    findShortestPath(originId, destinationId, penalty = -1.0) {
        const transferPenalty = penalty >= 0.0 ? penalty : this.defaultTransferPenalty;

        const result = {
            origin: originId,
            destination: destinationId,
            totalTimeMin: Infinity,
            totalDistanceKm: 0.0,
            transferCount: 0,
            totalPenaltyMin: 0.0,
            legs: [],
            pathNodes: [],
            isReachable: false
        };

        if (!this.graph.nodes.has(originId) || !this.graph.nodes.has(destinationId)) {
            return result;
        }

        if (originId === destinationId) {
            result.totalTimeMin = 0.0;
            result.pathNodes = [originId];
            result.isReachable = true;
            return result;
        }

        const pq = new MinPriorityQueue();
        const distances = new Map(); // key: "stationId:mode" -> minCost
        const predecessors = new Map(); // key: "stationId:mode" -> record

        // State: mode -1 (start), 0 (bus), 1 (train)
        const startKey = `${originId}:-1`;
        distances.set(startKey, 0.0);
        pq.push({ cost: 0.0, u: originId, mode: -1 });

        let bestTerminalKey = null;
        let minDestCost = Infinity;

        while (!pq.isEmpty()) {
            const curr = pq.pop();
            const currKey = `${curr.u}:${curr.mode}`;

            if (curr.cost > (distances.get(currKey) || Infinity)) continue;

            if (curr.u === destinationId) {
                if (curr.cost < minDestCost) {
                    minDestCost = curr.cost;
                    bestTerminalKey = currKey;
                }
                continue;
            }

            for (const edge of this.graph.getNeighbors(curr.u)) {
                const edgeMode = edge.mode === "TRAIN" ? 1 : 0;
                const nextKey = `${edge.target}:${edgeMode}`;

                let isTransfer = false;
                let transferDelay = 0.0;

                if (curr.mode !== -1 && curr.mode !== edgeMode) {
                    isTransfer = true;
                    transferDelay = transferPenalty;
                }

                const effectiveEdgeTime = edge.getEffectiveTravelTime();
                const newCost = curr.cost + effectiveEdgeTime + transferDelay;

                if (newCost < (distances.get(nextKey) || Infinity)) {
                    distances.set(nextKey, newCost);
                    predecessors.set(nextKey, {
                        prevU: curr.u,
                        prevMode: curr.mode,
                        edgeUsed: edge,
                        isTransfer: isTransfer,
                        transferDelay: transferDelay
                    });
                    pq.push({ cost: newCost, u: edge.target, mode: edgeMode });
                }
            }
        }

        if (!bestTerminalKey) {
            return result;
        }

        // Backtrack optimal path
        const legs = [];
        const pathNodes = [destinationId];
        let currKey = bestTerminalKey;
        let totalDistance = 0.0;
        let transferCount = 0;
        let totalPenalty = 0.0;

        while (predecessors.has(currKey)) {
            const rec = predecessors.get(currKey);
            if (rec.isTransfer) {
                transferCount++;
                totalPenalty += rec.transferDelay;
            }

            legs.push({
                source: rec.edgeUsed.source,
                target: rec.edgeUsed.target,
                mode: rec.edgeUsed.mode,
                distanceKm: rec.edgeUsed.distanceKm,
                travelTimeMin: rec.edgeUsed.getEffectiveTravelTime(),
                isTransferBefore: rec.isTransfer,
                transferPenaltyMin: rec.transferDelay
            });

            totalDistance += rec.edgeUsed.distanceKm;
            pathNodes.push(rec.prevU);
            currKey = `${rec.prevU}:${rec.prevMode}`;
        }

        legs.reverse();
        pathNodes.reverse();

        result.totalTimeMin = Math.round(minDestCost * 10) / 10;
        result.totalDistanceKm = Math.round(totalDistance * 10) / 10;
        result.transferCount = transferCount;
        result.totalPenaltyMin = totalPenalty;
        result.legs = legs;
        result.pathNodes = pathNodes;
        result.isReachable = true;

        return result;
    }
}

// --- 5. DEMAND SIMULATOR & EMERGENCY ---
class DemandSimulator {
    constructor(graph) {
        this.graph = graph;
    }

    generateTrips(windowType, count = 300) {
        const trips = [];
        const allIds = Array.from(this.graph.nodes.keys());
        const residential = [];
        const commercial = [];

        for (const [id, node] of this.graph.nodes.entries()) {
            const z = (node.zone || "").toLowerCase();
            if (z === "residential") residential.push(id);
            else commercial.push(id);
        }

        const resList = residential.length > 0 ? residential : allIds;
        const comList = commercial.length > 0 ? commercial : allIds;

        for (let i = 0; i < count; i++) {
            let u, v;
            const rand = Math.random();

            if (windowType === "morning") {
                u = rand < 0.8 ? resList[Math.floor(Math.random() * resList.length)] : allIds[Math.floor(Math.random() * allIds.length)];
                v = rand < 0.8 ? comList[Math.floor(Math.random() * comList.length)] : allIds[Math.floor(Math.random() * allIds.length)];
            } else if (windowType === "evening") {
                u = rand < 0.8 ? comList[Math.floor(Math.random() * comList.length)] : allIds[Math.floor(Math.random() * allIds.length)];
                v = rand < 0.8 ? resList[Math.floor(Math.random() * resList.length)] : allIds[Math.floor(Math.random() * allIds.length)];
            } else {
                u = allIds[Math.floor(Math.random() * allIds.length)];
                v = allIds[Math.floor(Math.random() * allIds.length)];
            }

            while (v === u) {
                v = allIds[Math.floor(Math.random() * allIds.length)];
            }

            trips.push({ origin: u, dest: v });
        }
        return trips;
    }

    simulateTimeWindow(windowType, count = 300) {
        this.graph.resetAllCongestion();
        const trips = this.generateTrips(windowType, count);
        const router = new MultiModalDijkstra(this.graph);

        let routed = 0;
        let totalTime = 0.0;
        let totalTransfers = 0;

        for (const trip of trips) {
            const res = router.findShortestPath(trip.origin, trip.dest);
            if (!res.isReachable) continue;

            routed++;
            totalTime += res.totalTimeMin;
            totalTransfers += res.transferCount;

            // Feedback flow into edges
            for (const leg of res.legs) {
                const neighbors = this.graph.getNeighbors(leg.source);
                for (const edge of neighbors) {
                    if (edge.target === leg.target && edge.mode === leg.mode) {
                        edge.addFlow(1);
                        break;
                    }
                }
            }
        }

        // Get all unique edges ranked by V/C ratio
        const allEdges = [];
        for (const edges of this.graph.adjList.values()) {
            for (const e of edges) {
                if (!e.routeId.endsWith("_REV")) {
                    allEdges.push(e);
                }
            }
        }

        allEdges.sort((a, b) => {
            const vca = a.capacity > 0 ? a.currentFlow / a.capacity : 0;
            const vcb = b.capacity > 0 ? b.currentFlow / b.capacity : 0;
            return vcb - vca;
        });

        return {
            routed: routed,
            total: count,
            avgTimeMin: routed > 0 ? Math.round((totalTime / routed) * 10) / 10 : 0,
            avgTransfers: routed > 0 ? Math.round((totalTransfers / routed) * 100) / 100 : 0,
            topBottlenecks: allEdges.slice(0, 5)
        };
    }

    simulateDisruption(routeId) {
        const revRouteId = routeId + "_REV";
        const trips = this.generateTrips("morning", 250);
        const router = new MultiModalDijkstra(this.graph);

        // 1. Baseline
        this.graph.resetAllCongestion();
        let normalRouted = 0;
        let normalTotalTime = 0;
        const baselinePaths = [];

        for (const trip of trips) {
            const res = router.findShortestPath(trip.origin, trip.dest);
            baselinePaths.push(res);
            if (res.isReachable) {
                normalRouted++;
                normalTotalTime += res.totalTimeMin;
            }
        }
        const normalAvg = normalRouted > 0 ? normalTotalTime / normalRouted : 0;

        // 2. Temporarily remove disrupted route
        const backupEdges = [];
        for (const [nodeId, edges] of this.graph.adjList.entries()) {
            for (let i = edges.length - 1; i >= 0; i--) {
                if (edges[i].routeId === routeId || edges[i].routeId === revRouteId) {
                    backupEdges.push({ nodeId, edge: edges.splice(i, 1)[0] });
                }
            }
        }

        // 3. Disrupted scenario
        this.graph.resetAllCongestion();
        let disRouted = 0;
        let disUnreachable = 0;
        let divertedCount = 0;
        let disTotalTime = 0;

        for (let i = 0; i < trips.length; i++) {
            const trip = trips[i];
            const res = router.findShortestPath(trip.origin, trip.dest);

            if (!res.isReachable) {
                disUnreachable++;
                continue;
            }

            disRouted++;
            disTotalTime += res.totalTimeMin;

            if (baselinePaths[i].isReachable) {
                const same = baselinePaths[i].pathNodes.join("-") === res.pathNodes.join("-");
                if (!same) divertedCount++;
            }

            for (const leg of res.legs) {
                for (const edge of this.graph.getNeighbors(leg.source)) {
                    if (edge.target === leg.target && edge.mode === leg.mode) {
                        edge.addFlow(1);
                        break;
                    }
                }
            }
        }

        const disAvg = disRouted > 0 ? disTotalTime / disRouted : 0;
        const addedDelay = Math.max(0, Math.round((disAvg - normalAvg) * 10) / 10);

        // 4. Restore edges
        for (const item of backupEdges) {
            this.graph.adjList.get(item.nodeId).push(item.edge);
        }
        this.graph.resetAllCongestion();

        return {
            routeId,
            diverted: divertedCount,
            addedDelayMin: addedDelay,
            unreachable: disUnreachable,
            normalAvgTime: Math.round(normalAvg * 10) / 10,
            disruptedAvgTime: Math.round(disAvg * 10) / 10
        };
    }
}

// --- 6. ALGORITHMIC PROFILER ---
class TransitProfiler {
    constructor(graph) {
        this.graph = graph;
    }

    runBenchmark(batchSize = 1000) {
        const nodeIds = Array.from(this.graph.nodes.keys());
        const queries = [];
        for (let i = 0; i < batchSize; i++) {
            let u = nodeIds[Math.floor(Math.random() * nodeIds.length)];
            let v = nodeIds[Math.floor(Math.random() * nodeIds.length)];
            while (v === u) v = nodeIds[Math.floor(Math.random() * nodeIds.length)];
            queries.push({ u, v });
        }

        const router = new MultiModalDijkstra(this.graph);

        // Cache warmup
        for (let i = 0; i < Math.min(20, batchSize); i++) {
            router.findShortestPath(queries[i].u, queries[i].v);
        }

        const latenciesUs = [];
        const tStartOverall = performance.now();

        for (let i = 0; i < batchSize; i++) {
            const t0 = performance.now();
            router.findShortestPath(queries[i].u, queries[i].v);
            const t1 = performance.now();
            latenciesUs.push((t1 - t0) * 1000.0); // Convert to microseconds
        }

        const tEndOverall = performance.now();
        const totalDurationSec = (tEndOverall - tStartOverall) / 1000.0;

        latenciesUs.sort((a, b) => a - b);
        const sum = latenciesUs.reduce((a, b) => a + b, 0);
        const mean = Math.round(sum / latenciesUs.length);
        const min = Math.round(latenciesUs[0]);
        const max = Math.round(latenciesUs[latenciesUs.length - 1]);
        const median = Math.round(latenciesUs[Math.floor(latenciesUs.length / 2)]);
        const p95 = Math.round(latenciesUs[Math.floor(latenciesUs.length * 0.95)]);
        const p99 = Math.round(latenciesUs[Math.floor(latenciesUs.length * 0.99)]);
        const qps = totalDurationSec > 0 ? Math.round(batchSize / totalDurationSec) : 0;

        return { batchSize, mean, min, max, median, p95, p99, qps };
    }
}

// --- 7. INTERACTIVE SVG MAP RENDERER ---
class InteractiveMapRenderer {
    constructor(svgElement, graph) {
        this.svg = svgElement;
        this.graph = graph;
        this.tooltip = document.getElementById("map-tooltip");

        // Viewport transform state
        this.zoom = 1.0;
        this.panX = 0;
        this.panY = 0;
        this.isDragging = false;
        this.dragStartX = 0;
        this.dragStartY = 0;

        this.highlightRoute = null;
        this.showCongestion = true;

        this.canvasW = 1200;
        this.canvasH = 880;
        this.calcBounds();
        this.bindEvents();
    }

    calcBounds() {
        let minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
        for (const node of this.graph.nodes.values()) {
            minX = Math.min(minX, node.x);
            maxX = Math.max(maxX, node.x);
            minY = Math.min(minY, node.y);
            maxY = Math.max(maxY, node.y);
        }

        this.minX = minX;
        this.maxX = maxX;
        this.minY = minY;
        this.maxY = maxY;
        this.padX = 120;
        this.padY = 130;
        this.drawW = this.canvasW - 2 * this.padX;
        this.drawH = this.canvasH - 2 * this.padY - 40;
    }

    toScreen(x, y) {
        const nx = (x - this.minX) / (this.maxX - this.minX);
        const ny = (y - this.minY) / (this.maxY - this.minY);
        const sx = this.padX + nx * this.drawW;
        const sy = (this.canvasH - this.padY) - ny * this.drawH; // Invert Y
        return { x: sx, y: sy };
    }

    render() {
        while (this.svg.firstChild) {
            this.svg.removeChild(this.svg.firstChild);
        }

        // Root zoom/pan group
        const rootG = document.createElementNS("http://www.w3.org/2000/svg", "g");
        rootG.setAttribute("id", "map-root-group");
        rootG.setAttribute("transform", `translate(${this.panX}, ${this.panY}) scale(${this.zoom})`);
        this.svg.appendChild(rootG);

        // 1. Grid Background
        const gridG = document.createElementNS("http://www.w3.org/2000/svg", "g");
        for (let gx = 0; gx < this.canvasW; gx += 80) {
            const line = document.createElementNS("http://www.w3.org/2000/svg", "line");
            line.setAttribute("x1", gx);
            line.setAttribute("y1", 0);
            line.setAttribute("x2", gx);
            line.setAttribute("y2", this.canvasH);
            line.setAttribute("stroke", "#172237");
            line.setAttribute("stroke-width", "1");
            line.setAttribute("stroke-dasharray", "3,3");
            gridG.appendChild(line);
        }
        for (let gy = 0; gy < this.canvasH; gy += 80) {
            const line = document.createElementNS("http://www.w3.org/2000/svg", "line");
            line.setAttribute("x1", 0);
            line.setAttribute("y1", gy);
            line.setAttribute("x2", this.canvasW);
            line.setAttribute("y2", gy);
            line.setAttribute("stroke", "#172237");
            line.setAttribute("stroke-width", "1");
            line.setAttribute("stroke-dasharray", "3,3");
            gridG.appendChild(line);
        }
        rootG.appendChild(gridG);

        // 2. SVG Defs (Markers & Filters)
        const defs = document.createElementNS("http://www.w3.org/2000/svg", "defs");
        defs.innerHTML = `
            <filter id="neonGlow" x="-50%" y="-50%" width="200%" height="200%">
                <feGaussianBlur stdDeviation="4.5" result="blur" />
                <feMerge>
                    <feMergeNode in="blur" />
                    <feMergeNode in="SourceGraphic" />
                </feMerge>
            </filter>
            <marker id="arrow-neon" viewBox="0 0 10 10" refX="22" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
                <path d="M 0 1 L 10 5 L 0 9 z" fill="#10B981" />
            </marker>
        `;
        rootG.appendChild(defs);

        // 3. Render Edges
        const edgesG = document.createElementNS("http://www.w3.org/2000/svg", "g");
        edgesG.setAttribute("id", "edges-layer");

        const seenEdges = new Set();

        for (const [srcId, edgeList] of this.graph.adjList.entries()) {
            for (const edge of edgeList) {
                const uNode = this.graph.getNode(edge.source);
                const vNode = this.graph.getNode(edge.target);
                if (!uNode || !vNode) continue;

                const p1 = this.toScreen(uNode.x, uNode.y);
                const p2 = this.toScreen(vNode.x, vNode.y);

                const dx = p2.x - p1.x;
                const dy = p2.y - p1.y;
                const len = Math.hypot(dx, dy);
                const offX = len > 1e-4 ? (-dy / len) * 3.5 : 0;
                const offY = len > 1e-4 ? (dx / len) * 3.5 : 0;

                const x1 = p1.x + offX;
                const y1 = p1.y + offY;
                const x2 = p2.x + offX;
                const y2 = p2.y + offY;

                let strokeColor = edge.mode === "TRAIN" ? "#EF4444" : "#3B82F6";
                let strokeDash = edge.mode === "TRAIN" ? "none" : "6,4";
                let strokeWidth = edge.mode === "TRAIN" ? 3.5 : 2.5;

                if (this.showCongestion) {
                    const vc = edge.capacity > 0 ? edge.currentFlow / edge.capacity : 0;
                    if (vc < 0.6) strokeColor = "#10B981";
                    else if (vc < 0.85) strokeColor = "#F59E0B";
                    else strokeColor = "#EF4444";
                }

                const line = document.createElementNS("http://www.w3.org/2000/svg", "line");
                line.setAttribute("x1", x1.toFixed(1));
                line.setAttribute("y1", y1.toFixed(1));
                line.setAttribute("x2", x2.toFixed(1));
                line.setAttribute("y2", y2.toFixed(1));
                line.setAttribute("stroke", strokeColor);
                line.setAttribute("stroke-width", strokeWidth);
                line.setAttribute("stroke-dasharray", strokeDash);
                line.setAttribute("stroke-linecap", "round");
                line.setAttribute("opacity", "0.75");
                line.classList.add("transit-edge");

                // Tooltip on Hover
                line.addEventListener("mouseenter", (e) => {
                    const vc = edge.capacity > 0 ? Math.round((edge.currentFlow / edge.capacity) * 100) : 0;
                    this.showTooltip(e, `
                        <strong>Route: ${edge.routeId} (${edge.mode})</strong><br>
                        ${uNode.name} &rarr; ${vNode.name}<br>
                        Distance: ${edge.distanceKm} km | Base Time: ${edge.baseTimeMin}m<br>
                        Congested Time: ${edge.getEffectiveTravelTime()}m<br>
                        Volume/Capacity: ${edge.currentFlow} / ${edge.capacity} (${vc}%)
                    `);
                });
                line.addEventListener("mousemove", (e) => this.moveTooltip(e));
                line.addEventListener("mouseleave", () => this.hideTooltip());

                edgesG.appendChild(line);
            }
        }
        rootG.appendChild(edgesG);

        // 4. Render Active Highlighted Route (if exists)
        if (this.highlightRoute && this.highlightRoute.isReachable) {
            const activeG = document.createElementNS("http://www.w3.org/2000/svg", "g");
            for (const leg of this.highlightRoute.legs) {
                const uNode = this.graph.getNode(leg.source);
                const vNode = this.graph.getNode(leg.target);
                if (!uNode || !vNode) continue;

                const p1 = this.toScreen(uNode.x, uNode.y);
                const p2 = this.toScreen(vNode.x, vNode.y);

                const line = document.createElementNS("http://www.w3.org/2000/svg", "line");
                line.setAttribute("x1", p1.x.toFixed(1));
                line.setAttribute("y1", p1.y.toFixed(1));
                line.setAttribute("x2", p2.x.toFixed(1));
                line.setAttribute("y2", p2.y.toFixed(1));
                line.setAttribute("stroke", "#10B981");
                line.setAttribute("stroke-width", "6");
                line.setAttribute("stroke-linecap", "round");
                line.setAttribute("filter", "url(#neonGlow)");
                line.setAttribute("marker-end", "url(#arrow-neon)");
                line.classList.add("active-pulse-route");

                activeG.appendChild(line);
            }
            rootG.appendChild(activeG);
        }

        // 5. Render Stations
        const stationsG = document.createElementNS("http://www.w3.org/2000/svg", "g");
        stationsG.setAttribute("id", "stations-layer");

        for (const node of this.graph.nodes.values()) {
            const p = this.toScreen(node.x, node.y);
            const g = document.createElementNS("http://www.w3.org/2000/svg", "g");
            g.setAttribute("transform", `translate(${p.x.toFixed(1)}, ${p.y.toFixed(1)})`);
            g.classList.add("station-node");
            g.dataset.id = node.id;

            // Station Symbol
            if (node.type === "INTERCHANGE_HUB") {
                const halo = document.createElementNS("http://www.w3.org/2000/svg", "circle");
                halo.setAttribute("r", "16");
                halo.setAttribute("fill", "#F59E0B");
                halo.setAttribute("opacity", "0.2");
                g.appendChild(halo);

                const poly = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
                poly.setAttribute("points", "0,-12 12,0 0,12 -12,0");
                poly.setAttribute("fill", "#F59E0B");
                poly.setAttribute("stroke", "#FFFFFF");
                poly.setAttribute("stroke-width", "2");
                g.appendChild(poly);
            } else if (node.type === "TRAIN_STATION") {
                const rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
                rect.setAttribute("x", "-8");
                rect.setAttribute("y", "-8");
                rect.setAttribute("width", "16");
                rect.setAttribute("height", "16");
                rect.setAttribute("rx", "3");
                rect.setAttribute("fill", "#EF4444");
                rect.setAttribute("stroke", "#FFFFFF");
                rect.setAttribute("stroke-width", "1.5");
                g.appendChild(rect);
            } else {
                const circ = document.createElementNS("http://www.w3.org/2000/svg", "circle");
                circ.setAttribute("r", "7");
                circ.setAttribute("fill", "#3B82F6");
                circ.setAttribute("stroke", "#FFFFFF");
                circ.setAttribute("stroke-width", "1.5");
                g.appendChild(circ);
            }

            // Station Pill Label
            const labelBg = document.createElementNS("http://www.w3.org/2000/svg", "rect");
            const labelW = node.name.length * 6.5 + 20;
            labelBg.setAttribute("x", "10");
            labelBg.setAttribute("y", "-11");
            labelBg.setAttribute("width", labelW.toFixed(0));
            labelBg.setAttribute("height", "18");
            labelBg.setAttribute("rx", "4");
            labelBg.setAttribute("fill", "#0F172A");
            labelBg.setAttribute("fill-opacity", "0.9");
            labelBg.setAttribute("stroke", "#334155");
            labelBg.setAttribute("stroke-width", "1");
            g.appendChild(labelBg);

            const txt = document.createElementNS("http://www.w3.org/2000/svg", "text");
            txt.setAttribute("x", "16");
            txt.setAttribute("y", "2");
            txt.setAttribute("fill", "#F8FAFC");
            txt.setAttribute("font-size", "10.5");
            txt.setAttribute("font-weight", "bold");
            txt.setAttribute("font-family", "'Plus Jakarta Sans', sans-serif");
            txt.textContent = `${node.id}: ${node.name}`;
            g.appendChild(txt);

            // Click Station to Select Origin / Destination
            g.addEventListener("click", () => {
                window.onStationClicked(node.id);
            });

            // Tooltip
            g.addEventListener("mouseenter", (e) => {
                this.showTooltip(e, `
                    <strong>${node.id}: ${node.name}</strong><br>
                    Type: ${node.type}<br>
                    Zone: ${node.zone}<br>
                    <em>Click to set as Origin / Destination</em>
                `);
            });
            g.addEventListener("mousemove", (e) => this.moveTooltip(e));
            g.addEventListener("mouseleave", () => this.hideTooltip());

            stationsG.appendChild(g);
        }
        rootG.appendChild(stationsG);
    }

    showTooltip(e, html) {
        this.tooltip.innerHTML = html;
        this.tooltip.style.display = "block";
        this.moveTooltip(e);
    }

    moveTooltip(e) {
        const offset = 14;
        this.tooltip.style.left = (e.clientX + offset) + "px";
        this.tooltip.style.top = (e.clientY + offset) + "px";
    }

    hideTooltip() {
        this.tooltip.style.display = "none";
    }

    bindEvents() {
        // Drag to pan
        const viewport = document.getElementById("svg-viewport");

        viewport.addEventListener("mousedown", (e) => {
            if (e.target.closest(".station-node")) return;
            this.isDragging = true;
            this.dragStartX = e.clientX - this.panX;
            this.dragStartY = e.clientY - this.panY;
        });

        window.addEventListener("mousemove", (e) => {
            if (!this.isDragging) return;
            this.panX = e.clientX - this.dragStartX;
            this.panY = e.clientY - this.dragStartY;
            this.updateTransform();
        });

        window.addEventListener("mouseup", () => {
            this.isDragging = false;
        });

        // Wheel zoom
        viewport.addEventListener("wheel", (e) => {
            e.preventDefault();
            const zoomFactor = e.deltaY < 0 ? 1.15 : 0.85;
            this.zoomAtPoint(e.clientX, e.clientY, zoomFactor);
        }, { passive: false });

        // HUD buttons
        document.getElementById("btn-zoom-in").addEventListener("click", () => {
            this.zoom *= 1.25;
            this.updateTransform();
        });

        document.getElementById("btn-zoom-out").addEventListener("click", () => {
            this.zoom = Math.max(0.4, this.zoom / 1.25);
            this.updateTransform();
        });

        document.getElementById("btn-zoom-reset").addEventListener("click", () => {
            this.zoom = 1.0;
            this.panX = 0;
            this.panY = 0;
            this.updateTransform();
        });
    }

    zoomAtPoint(clientX, clientY, factor) {
        const rect = this.svg.getBoundingClientRect();
        const mouseX = clientX - rect.left;
        const mouseY = clientY - rect.top;

        const newZoom = Math.max(0.4, Math.min(3.5, this.zoom * factor));
        this.panX = mouseX - (mouseX - this.panX) * (newZoom / this.zoom);
        this.panY = mouseY - (mouseY - this.panY) * (newZoom / this.zoom);
        this.zoom = newZoom;
        this.updateTransform();
    }

    updateTransform() {
        const rootG = document.getElementById("map-root-group");
        if (rootG) {
            rootG.setAttribute("transform", `translate(${this.panX}, ${this.panY}) scale(${this.zoom})`);
        }
    }
}

// --- 8. APPLICATION CONTROLLER & UI WIRE-UP ---
document.addEventListener("DOMContentLoaded", () => {
    const graph = new TransitGraph();
    const router = new MultiModalDijkstra(graph);
    const simulator = new DemandSimulator(graph);
    const profiler = new TransitProfiler(graph);

    const svgElement = document.getElementById("transit-svg");
    const mapRenderer = new InteractiveMapRenderer(svgElement, graph);
    mapRenderer.render();

    // Populate dropdowns
    const originSel = document.getElementById("select-origin");
    const destSel = document.getElementById("select-dest");
    const disruptSel = document.getElementById("select-disrupted-route");

    for (const [id, node] of graph.nodes.entries()) {
        const opt1 = document.createElement("option");
        opt1.value = id;
        opt1.textContent = `${id} - ${node.name} (${node.type === "INTERCHANGE_HUB" ? "HUB" : node.type})`;
        originSel.appendChild(opt1);

        const opt2 = document.createElement("option");
        opt2.value = id;
        opt2.textContent = `${id} - ${node.name} (${node.type === "INTERCHANGE_HUB" ? "HUB" : node.type})`;
        destSel.appendChild(opt2);
    }

    // Default select ST01 -> ST08
    originSel.value = "ST01";
    destSel.value = "ST08";

    // Populate routes in disruption dropdown
    for (const rt of RAW_ROUTES) {
        const opt = document.createElement("option");
        opt.value = rt.route_id;
        opt.textContent = `${rt.route_id} (${rt.mode}): ${rt.source} <-> ${rt.target} (${rt.distance_km}km, ${rt.base_time_min}m)`;
        disruptSel.appendChild(opt);
    }

    // Tab Navigation
    const tabButtons = document.querySelectorAll(".tab-btn");
    const tabPanes = document.querySelectorAll(".tab-pane");

    tabButtons.forEach(btn => {
        btn.addEventListener("click", () => {
            tabButtons.forEach(b => b.classList.remove("active"));
            tabPanes.forEach(p => p.classList.remove("active"));
            btn.classList.add("active");
            const targetId = btn.getAttribute("data-tab");
            document.getElementById(targetId).classList.add("active");
        });
    });

    // Quick Station Click on Map
    let stationClickToggle = false;
    window.onStationClicked = (stId) => {
        if (!stationClickToggle) {
            originSel.value = stId;
            stationClickToggle = true;
        } else {
            destSel.value = stId;
            stationClickToggle = false;
            // Auto trigger route calculation if on planner tab
            document.getElementById("btn-plan-route").click();
        }
    };

    // Swap stations button
    document.getElementById("btn-swap-stations").addEventListener("click", () => {
        const tmp = originSel.value;
        originSel.value = destSel.value;
        destSel.value = tmp;
    });

    // 1. Plan Route Button Handler
    document.getElementById("btn-plan-route").addEventListener("click", () => {
        const orig = originSel.value;
        const dest = destSel.value;
        const pref = document.querySelector('input[name="routing-pref"]:checked').value;

        if (orig === dest) {
            alert("Origin and Destination stations must be different.");
            return;
        }

        const resultsCard = document.getElementById("planner-results");
        const comparisonBox = document.getElementById("comparison-details");
        const itineraryList = document.getElementById("itinerary-steps");

        if (pref === "compare") {
            const fastRes = router.findShortestPath(orig, dest, 6.0);
            const directRes = router.findShortestPath(orig, dest, 35.0);

            comparisonBox.style.display = "block";
            comparisonBox.innerHTML = `
                <strong>Pareto Trade-Off Comparison:</strong><br>
                &bull; <strong>Fastest Time</strong>: ${fastRes.totalTimeMin} mins, ${fastRes.transferCount} transfer(s), ${fastRes.totalDistanceKm} km.<br>
                &bull; <strong>Min Transfers</strong>: ${directRes.totalTimeMin} mins, ${directRes.transferCount} transfer(s), ${directRes.totalDistanceKm} km.<br>
                ${fastRes.transferCount !== directRes.transferCount 
                    ? `<em>Trade-off: Saving ${fastRes.transferCount - directRes.transferCount} transfer(s) adds +${(directRes.totalTimeMin - fastRes.totalTimeMin).toFixed(1)} mins duration.</em>` 
                    : `<em>Both criteria select the identical optimal route!</em>`}
            `;

            renderItinerary(fastRes);
            mapRenderer.highlightRoute = fastRes;
        } else {
            comparisonBox.style.display = "none";
            const penalty = pref === "transfers" ? 35.0 : 6.0;
            const res = router.findShortestPath(orig, dest, penalty);
            renderItinerary(res);
            mapRenderer.highlightRoute = res;
        }

        mapRenderer.render();
        resultsCard.style.display = "flex";
    });

    function renderItinerary(res) {
        document.getElementById("res-time").textContent = `${res.totalTimeMin}m`;
        document.getElementById("res-dist").textContent = `${res.totalDistanceKm}km`;
        document.getElementById("res-transfers").textContent = `${res.transferCount}`;

        const list = document.getElementById("itinerary-steps");
        list.innerHTML = "";

        if (!res.isReachable) {
            list.innerHTML = `<div class="itinerary-step text-danger">No path found between selected stations.</div>`;
            return;
        }

        res.legs.forEach((leg, idx) => {
            const u = graph.getNode(leg.source);
            const v = graph.getNode(leg.target);

            if (leg.isTransferBefore) {
                const step = document.createElement("div");
                step.className = "itinerary-step";
                step.innerHTML = `
                    <span class="step-badge transfer">TRANSFER</span>
                    <div class="step-content">
                        <div class="step-title">Change lines at ${u ? u.name : leg.source}</div>
                        <div class="step-sub">+${leg.transferPenaltyMin} mins transfer penalty</div>
                    </div>
                `;
                list.appendChild(step);
            }

            const step = document.createElement("div");
            step.className = "itinerary-step";
            const badgeClass = leg.mode === "TRAIN" ? "train" : "bus";
            step.innerHTML = `
                <span class="step-badge ${badgeClass}">${leg.mode}</span>
                <div class="step-content">
                    <div class="step-title">${u ? u.name : leg.source} &rarr; ${v ? v.name : leg.target}</div>
                    <div class="step-sub">${leg.distanceKm} km &bull; ${leg.travelTimeMin} mins</div>
                </div>
            `;
            list.appendChild(step);
        });
    }

    // 2. Demand Slider & Simulation Handler
    const paxRange = document.getElementById("range-passengers");
    const paxLbl = document.getElementById("lbl-passengers");
    paxRange.addEventListener("input", () => {
        paxLbl.textContent = `${paxRange.value} pax`;
    });

    document.getElementById("btn-run-demand").addEventListener("click", () => {
        const windowType = document.getElementById("select-window").value;
        const pax = parseInt(paxRange.value, 10);
        const result = simulator.simulateTimeWindow(windowType, pax);

        document.getElementById("dem-routed").textContent = `${result.routed} / ${result.total}`;
        document.getElementById("dem-avg-time").textContent = `${result.avgTimeMin}m`;
        document.getElementById("dem-avg-transfers").textContent = `${result.avgTransfers}`;

        const table = document.getElementById("bottleneck-table");
        table.innerHTML = "";

        result.topBottlenecks.forEach(edge => {
            const u = graph.getNode(edge.source);
            const v = graph.getNode(edge.target);
            const vc = edge.capacity > 0 ? (edge.currentFlow / edge.capacity) : 0;
            const vcClass = vc > 0.85 ? "vc-high" : (vc > 0.6 ? "vc-mod" : "vc-low");

            const row = document.createElement("div");
            row.className = "bottleneck-row";
            row.innerHTML = `
                <div>
                    <strong>${edge.routeId} (${edge.mode})</strong><br>
                    <span style="color: #94A3B8;">${u ? u.name : edge.source} &rarr; ${v ? v.name : edge.target}</span>
                </div>
                <div style="text-align: right;">
                    <span class="vc-tag ${vcClass}">${Math.round(vc * 100)}%</span><br>
                    <span style="font-size: 0.7rem; color: #64748B;">${edge.baseTimeMin}m &rarr; ${edge.getEffectiveTravelTime()}m</span>
                </div>
            `;
            table.appendChild(row);
        });

        document.getElementById("demand-results").style.display = "flex";
        mapRenderer.showCongestion = document.getElementById("chk-congestion-heatmap").checked;
        mapRenderer.render();
    });

    document.getElementById("btn-reset-congestion").addEventListener("click", () => {
        graph.resetAllCongestion();
        document.getElementById("demand-results").style.display = "none";
        mapRenderer.render();
    });

    document.getElementById("chk-congestion-heatmap").addEventListener("change", (e) => {
        mapRenderer.showCongestion = e.target.checked;
        mapRenderer.render();
    });

    // 3. Emergency Disruption Handler
    document.getElementById("btn-simulate-disruption").addEventListener("click", () => {
        const routeId = disruptSel.value;
        const result = simulator.simulateDisruption(routeId);

        const card = document.getElementById("disruption-results");
        document.getElementById("disruption-status-badge").innerHTML = `
            <div style="background: rgba(239, 68, 68, 0.2); border: 1px solid #EF4444; color: #FCA5A5; padding: 6px 10px; border-radius: 6px; font-size: 0.8rem; font-weight: 600;">
                Route ${result.routeId} Blocked in Both Directions
            </div>
        `;

        document.getElementById("dis-diverted").textContent = `${result.diverted}`;
        document.getElementById("dis-delay").textContent = `+${result.addedDelayMin}m`;
        document.getElementById("dis-unreachable").textContent = `${result.unreachable}`;

        document.getElementById("disruption-bus-impact").innerHTML = `
            <div style="font-size: 0.78rem; color: #94A3B8; margin-top: 8px;">
                Normal Network Avg Time: <strong>${result.normalAvgTime}m</strong><br>
                Disrupted Network Avg Time: <strong style="color: #EF4444;">${result.disruptedAvgTime}m</strong>
            </div>
        `;

        card.style.display = "flex";
        mapRenderer.render();
    });

    document.getElementById("btn-restore-routes").addEventListener("click", () => {
        graph.init();
        document.getElementById("disruption-results").style.display = "none";
        mapRenderer.render();
    });

    // 4. DSA Profiler Handler
    const queryRange = document.getElementById("range-queries");
    const queryLbl = document.getElementById("lbl-queries");
    queryRange.addEventListener("input", () => {
        queryLbl.textContent = `${parseInt(queryRange.value, 10).toLocaleString()} queries`;
    });

    document.getElementById("btn-run-profile").addEventListener("click", () => {
        const batch = parseInt(queryRange.value, 10);
        const stats = profiler.runBenchmark(batch);

        document.getElementById("prof-mean").textContent = `${stats.mean} μs`;
        document.getElementById("prof-qps").textContent = `${stats.qps.toLocaleString()}`;
        document.getElementById("prof-p95").textContent = `${stats.p95} μs`;

        document.getElementById("prof-min").textContent = `${stats.min} μs`;
        document.getElementById("prof-median").textContent = `${stats.median} μs`;
        document.getElementById("prof-p99").textContent = `${stats.p99} μs`;
        document.getElementById("prof-max").textContent = `${stats.max} μs`;

        document.getElementById("profile-results").style.display = "flex";
    });
});
