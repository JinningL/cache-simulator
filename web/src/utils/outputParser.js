export function parseSimulatorOutput(text) {
  const accesses = [];
  const levels   = {};
  const overall  = {};

  const lines  = text.split('\n');
  let section  = null;

  for (let i = 0; i < lines.length; i++) {
    const line    = lines[i];
    const trimmed = line.trim();

    /* Section headers */
    if (trimmed.startsWith('=== Access Log ==='))  { section = 'access';  continue; }
    if (trimmed.startsWith('=== L1 Cache ==='))    { section = 'L1'; levels.L1 = {}; continue; }
    if (trimmed.startsWith('=== L2 Cache ==='))    { section = 'L2'; levels.L2 = {}; continue; }
    if (trimmed.startsWith('=== L3 Cache ==='))    { section = 'L3'; levels.L3 = {}; continue; }
    if (trimmed.startsWith('=== Overall ==='))     { section = 'overall'; continue; }
    if (trimmed.startsWith('==='))                 { section = null;      continue; }

    /* Access log */
    if (section === 'access') {
      const m = trimmed.match(/^([RW])\s+(0x[0-9a-f]+)\s+->\s+(.+)$/i);
      if (m) {
        accesses.push({
          op:     m[1],
          addr:   m[2],
          result: classifyResult(m[3]),
          detail: m[3],
        });
      }
    }

    /* Per-level stats */
    if (section === 'L1' || section === 'L2' || section === 'L3')
      parseLevelLine(trimmed, levels[section]);

    /* Overall stats */
    if (section === 'overall') {
      match(trimmed, /Total Accesses:\s+([\d,]+)/,    v => { overall.totalAccesses  = int(v); });
      match(trimmed, /Memory Accesses:\s+([\d,]+)/,   v => { overall.memoryAccesses = int(v); });
      match(trimmed, /AMAT \(approx\):\s+([\d.]+)/,   v => { overall.amat           = parseFloat(v); });

      /* "Active levels:  L1=2  L3=40  Mem=100 cycles" */
      const mLat = trimmed.match(/^Latencies:(.+)$/);
      if (mLat) {
        const latencies = [];
        const parts = mLat[1].trim().split(/\s+/);
        let memLatency = null;
        for (const p of parts) {
          const kv = p.match(/^([A-Za-z0-9]+)=(\d+)/);
          if (!kv) continue;
          if (kv[1] === 'Mem') { memLatency = parseInt(kv[2]); continue; }
          latencies.push({ name: kv[1], cycles: parseInt(kv[2]) });
        }
        overall.latencies  = latencies;
        overall.memLatency = memLatency;
      }

      /* "AMAT formula:  L1(1) + mr_L1*(…)" */
      const mFormula = trimmed.match(/^AMAT formula:\s+(.+)$/);
      if (mFormula) overall.amtFormula = mFormula[1].trim();
    }
  }

  return { accesses, levels, overall };
}

function parseLevelLine(line, obj) {
  const pairs = [
    [/Accesses:\s+(\d+)/,         'accesses'],
    [/Reads:\s+(\d+)/,            'reads'],
    [/Writes:\s+(\d+)/,           'writes'],
    [/Hits:\s+(\d+)/,             'hits'],
    [/Misses:\s+(\d+)/,           'misses'],
    [/Evictions:\s+(\d+)/,        'evictions'],
    [/Dirty Writebacks:\s+(\d+)/, 'dirtyWritebacks'],
    [/Hit Rate:\s+([\d.]+)%/,     'hitRate'],
    [/Miss Rate:\s+([\d.]+)%/,    'missRate'],
  ];
  for (const [re, key] of pairs) {
    const m = line.match(re);
    if (m) obj[key] = key.endsWith('Rate') ? parseFloat(m[1]) : parseInt(m[1]);
  }
}

function classifyResult(detail) {
  if (/L1 HIT/.test(detail)) return 'L1';
  if (/L2 HIT/.test(detail)) return 'L2';
  if (/L3 HIT/.test(detail)) return 'L3';
  return 'MEMORY';
}

function match(str, re, fn) {
  const m = str.match(re);
  if (m) fn(m[1]);
}

function int(s) { return parseInt(s.replace(',', ''), 10); }
