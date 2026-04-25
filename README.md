# cache-simulator

A modular, configurable multi-level cache simulator — C backend with an optional React web UI.

## Features

- Three-level hierarchy: L1 → L2 → L3 → Main Memory
- Configurable size and associativity per level
- Write-back + write-allocate policy with dirty bits
- LRU and FIFO replacement policies
- Per-level statistics and approximate AMAT
- **Web UI** — beginner-friendly configuration form, live access timeline, and stats visualisation

---

## Web UI (React + Vite)

The `web/` directory contains a React + Vite frontend with a tiny Express backend that executes the C binary.

Product site: https://cache-simulator-wv04.onrender.com/

### Quick start

```bash
# 1. Build the C simulator (project root)
make

# 2. Install web dependencies
cd web
npm install

# 3. Start both servers (Vite dev server + Express API)
npm run dev
```

Then open **http://localhost:5173** in your browser.

### What the UI does

| Panel | Content |
|-------|---------|
| Left — Configuration | L1/L2/L3 size (KB/MB), associativity, block size, LRU/FIFO policy |
| Left — Memory Trace | Paste trace text or upload a `.txt` file |
| Right — Results | Generated CLI command, per-level statistics, hit-rate bars, access timeline |

### How sizes and associativity map to the CLI

| UI selection | Bytes sent | Flag |
|---|---|---|
| 32 KB | 32768 | `--l1-size 32768` |
| 256 KB | 262144 | `--l2-size 262144` |
| 8 MB | 8388608 | `--l3-size 8388608` |
| Direct-mapped | 1 | `--l1-assoc 1` |
| 4-way | 4 | `--l1-assoc 4` |
| Fully Associative | cache_size / block_size | `--l1-assoc <num_lines>` |

### web/ structure

```
web/
├── server.js          Express API server (runs the C binary, port 3001)
├── vite.config.js     Vite dev server with /api proxy to :3001
├── src/
│   ├── App.jsx / App.css
│   ├── components/
│   │   ├── ConfigPanel.jsx   Cache configuration form
│   │   ├── TraceInput.jsx    Trace paste / upload
│   │   ├── StatsDisplay.jsx  Per-level statistics + AMAT
│   │   └── AccessTimeline.jsx  Colour-coded access log
│   └── utils/
│       ├── commandBuilder.js  Form → CLI args conversion
│       ├── validation.js      Input validation rules
│       └── outputParser.js    C simulator text → structured data
└── package.json
```

---

## How Cache Address Decoding Works

A byte address is split into three fields:

```
| <-- tag bits --> | <-- index bits --> | <-- offset bits --> |
```

| Field  | Bits              | Meaning                                      |
|--------|-------------------|----------------------------------------------|
| offset | log2(block_size)  | Byte position within the block               |
| index  | log2(num_sets)    | Which set (row) in the cache to look in      |
| tag    | remaining bits    | Identifies which block occupies the set slot |

```
offset_bits   = log2(block_size)
index_bits    = log2(num_sets)
block_address = address >> offset_bits
index         = block_address  &  (num_sets - 1)
tag           = block_address  >> index_bits
```

---

## Cache Types

| Type             | Associativity          | Description                                      |
|------------------|------------------------|--------------------------------------------------|
| Direct-mapped    | 1                      | Each block maps to exactly one line              |
| Set-associative  | n (2, 4, 8, …)         | Each block maps to one set; n lines compete      |
| Fully associative| num_lines (all sets=1) | A block can go anywhere in the cache             |

Set a level's `--l1-assoc` (etc.) to `1` for direct-mapped, `num_lines` for fully associative.

---

## Write-Back vs Write-Through

| Policy      | On write hit              | On eviction of dirty line          |
|-------------|---------------------------|------------------------------------|
| Write-back  | Update cache only (dirty) | Write to next level at eviction    |
| Write-through| Write cache AND memory   | No extra work needed at eviction   |

**This simulator uses write-back.** Modified lines are not written to the next level until they are evicted. The `dirty` bit tracks which lines need to be written back.

---

## The Dirty Bit

Each cache line has a `dirty` flag (0 or 1):

- `dirty = 0`: line contents match what is stored in the lower level/memory.
- `dirty = 1`: the line was written to while in cache; the lower level has stale data.

When a dirty line is evicted, its data is written back to the next cache level (or main memory if it is an L3 eviction) before the slot is reused.

---

## Building

```bash
make          # compile everything → produces ./cache_sim
make clean    # remove object files and binary
make run      # build and run with the default trace.txt and example config
```

Requires `gcc` and a POSIX shell. Only standard C11 libraries are used.

---

## Running

```
./cache_sim \
  --l1-size  32768     --l1-assoc 4  \
  --l2-size  262144    --l2-assoc 8  \
  --l3-size  8388608   --l3-assoc 16 \
  --block    64                       \
  --policy   LRU                      \
  trace.txt
```

### Options

| Flag          | Description                         | Example      |
|---------------|-------------------------------------|--------------|
| `--l1-size`   | L1 cache size in bytes              | `32768`      |
| `--l1-assoc`  | L1 associativity (ways)             | `4`          |
| `--l2-size`   | L2 cache size in bytes              | `262144`     |
| `--l2-assoc`  | L2 associativity                    | `8`          |
| `--l3-size`   | L3 cache size in bytes              | `8388608`    |
| `--l3-assoc`  | L3 associativity                    | `16`         |
| `--block`     | Block (cache line) size in bytes    | `64`         |
| `--policy`    | Replacement policy: `LRU` or `FIFO` | `LRU`        |
| (positional)  | Path to trace file                  | `trace.txt`  |

---

## Trace File Format

One memory access per line:

```
R 0x00001A3C
W 0x00001A40
R 0x00001A44
```

- `R` = read (load)
- `W` = write (store)
- Address is a hexadecimal byte address prefixed with `0x`
- Lines starting with `#` are treated as comments and ignored

---

## Sample Output

```
=== Configuration ===
  L1: 32768 bytes, 4-way
  L2: 262144 bytes, 8-way
  L3: 8388608 bytes, 16-way
  Block size: 64 bytes
  Policy: LRU
  Trace: trace.txt

=== Access Log ===
R 0x00001a3c -> L1 MISS, L2 MISS, L3 MISS, MEMORY
W 0x00001a40 -> L1 HIT
R 0x00001a44 -> L1 HIT
...

=== L1 Cache ===
  Accesses:             20
  Reads:                14   Writes:                6
  Hits:                 10   Misses:               10
  Evictions:             0   Dirty Writebacks:      0
  Hit Rate:          50.00%  Miss Rate:          50.00%

=== L2 Cache ===
  ...

=== Overall ===
  Total Accesses:       20
  Memory Accesses:       8
  AMAT (approx):      6.32 cycles

  Latencies used: L1=1, L2=10, L3=40, Mem=100
  Formula: L1 + mr_L1*(L2 + mr_L2*(L3 + mr_L3*Mem))
```

---

## File Structure

```
cache-simulator/
├── main.c         Entry point: parse args, run simulation loop, print stats
├── config.c/h     CLI argument parsing and SimConfig struct
├── cache.c/h      CacheLine/CacheSet/CacheLevel structs and operations
├── hierarchy.c/h  CacheHierarchy and multi-level access logic
├── policy.c/h     LRU and FIFO victim selection
├── trace.c/h      Trace file reader
├── stats.c/h      Statistics structs and formatted output
├── utils.c/h      Address decoding and integer log2
├── Makefile
├── trace.txt      Sample memory trace
└── README.md
```
