import express from 'express';
import cors from 'cors';
import { exec } from 'child_process';
import { writeFile, unlink } from 'fs/promises';
import { join, dirname } from 'path';
import { tmpdir } from 'os';
import { randomBytes } from 'crypto';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const BINARY = join(__dirname, '..', 'cache_sim');
const DIST_DIR = join(__dirname, 'dist');
const PORT = process.env.PORT || 3001;

const app = express();
app.use(cors());
app.use(express.json({ limit: '2mb' }));

app.post('/api/simulate', async (req, res) => {
  const { config: c, trace } = req.body;
  if (!c || !trace) return res.status(400).json({ error: 'Missing config or trace' });

  const id = randomBytes(6).toString('hex');
  const traceFile = join(tmpdir(), `cache_trace_${id}.txt`);

  try {
    await writeFile(traceFile, trace, 'utf8');

    /* Build CLI args from config */
    const args = [];

    if (c.l1Enabled)
      args.push(`--l1-size ${c.l1Size}`, `--l1-assoc ${c.l1Assoc}`, `--l1-latency ${c.l1Latency}`);
    else
      args.push('--no-l1');

    if (c.l2Enabled)
      args.push(`--l2-size ${c.l2Size}`, `--l2-assoc ${c.l2Assoc}`, `--l2-latency ${c.l2Latency}`);
    else
      args.push('--no-l2');

    if (c.l3Enabled)
      args.push(`--l3-size ${c.l3Size}`, `--l3-assoc ${c.l3Assoc}`, `--l3-latency ${c.l3Latency}`);
    else
      args.push('--no-l3');

    args.push(`--block ${c.blockSize}`, `--policy ${c.policy}`, `--mem-latency ${c.memLatency}`);
    args.push(traceFile);

    const cmd = `${BINARY} ${args.join(' ')}`;

    exec(cmd, { timeout: 15000 }, async (err, stdout, stderr) => {
      try { await unlink(traceFile); } catch {}

      if (err)
        return res.status(400).json({ error: (stderr || err.message || 'Simulation failed').trim() });

      res.json({ rawOutput: stdout, command: buildDisplayCmd(c) });
    });
  } catch (e) {
    try { await unlink(traceFile); } catch {}
    res.status(500).json({ error: e.message });
  }
});

function buildDisplayCmd(c) {
  const lines = ['./cache_sim \\'];
  if (c.l1Enabled)
    lines.push(`  --l1-size ${c.l1Size}  --l1-assoc ${c.l1Assoc}  --l1-latency ${c.l1Latency} \\`);
  else
    lines.push('  --no-l1 \\');
  if (c.l2Enabled)
    lines.push(`  --l2-size ${c.l2Size}  --l2-assoc ${c.l2Assoc}  --l2-latency ${c.l2Latency} \\`);
  else
    lines.push('  --no-l2 \\');
  if (c.l3Enabled)
    lines.push(`  --l3-size ${c.l3Size}  --l3-assoc ${c.l3Assoc}  --l3-latency ${c.l3Latency} \\`);
  else
    lines.push('  --no-l3 \\');
  lines.push(`  --block ${c.blockSize}  --policy ${c.policy}  --mem-latency ${c.memLatency} \\`);
  lines.push('  trace.txt');
  return lines.join('\n');
}

app.use(express.static(DIST_DIR));

app.get(/.*/, (_req, res) => {
  res.sendFile(join(DIST_DIR, 'index.html'));
});

app.listen(PORT, () => console.log(`Cache simulator server on port ${PORT}`));
