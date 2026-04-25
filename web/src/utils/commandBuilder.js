export function sizeToBytes(value, unit) {
  const n = parseFloat(value);
  if (unit === 'KB') return Math.round(n * 1024);
  if (unit === 'MB') return Math.round(n * 1024 * 1024);
  return Math.round(n);
}

export function assocToNumber(type, numLines) {
  if (type === 'direct') return 1;
  if (type === 'full')   return numLines;
  return parseInt(type, 10);
}

/*
 * Build the resolved config object from the raw form state.
 * Disabled levels still carry size/assoc (ignored by the binary)
 * but are flagged with enabled=false so the server can skip them.
 */
export function buildConfig(form) {
  const blockSize = parseInt(form.blockSize, 10);

  const l1Size  = sizeToBytes(form.l1SizeVal, form.l1SizeUnit);
  const l2Size  = sizeToBytes(form.l2SizeVal, form.l2SizeUnit);
  const l3Size  = sizeToBytes(form.l3SizeVal, form.l3SizeUnit);

  const l1Assoc = assocToNumber(form.l1Assoc, l1Size / blockSize);
  const l2Assoc = assocToNumber(form.l2Assoc, l2Size / blockSize);
  const l3Assoc = assocToNumber(form.l3Assoc, l3Size / blockSize);

  return {
    l1Enabled: form.l1Enabled, l1Size, l1Assoc, l1Latency: parseInt(form.l1Latency, 10) || 1,
    l2Enabled: form.l2Enabled, l2Size, l2Assoc, l2Latency: parseInt(form.l2Latency, 10) || 10,
    l3Enabled: form.l3Enabled, l3Size, l3Assoc, l3Latency: parseInt(form.l3Latency, 10) || 40,
    blockSize,
    policy: form.policy,
    memLatency: parseInt(form.memLatency, 10) || 100,
  };
}

/* Render the display CLI command string. */
export function buildDisplayCommand(cfg) {
  const lines = ['./cache_sim \\'];

  if (cfg.l1Enabled)
    lines.push(`  --l1-size ${cfg.l1Size}  --l1-assoc ${cfg.l1Assoc}  --l1-latency ${cfg.l1Latency} \\`);
  else
    lines.push('  --no-l1 \\');

  if (cfg.l2Enabled)
    lines.push(`  --l2-size ${cfg.l2Size}  --l2-assoc ${cfg.l2Assoc}  --l2-latency ${cfg.l2Latency} \\`);
  else
    lines.push('  --no-l2 \\');

  if (cfg.l3Enabled)
    lines.push(`  --l3-size ${cfg.l3Size}  --l3-assoc ${cfg.l3Assoc}  --l3-latency ${cfg.l3Latency} \\`);
  else
    lines.push('  --no-l3 \\');

  lines.push(`  --block ${cfg.blockSize}  --policy ${cfg.policy}  --mem-latency ${cfg.memLatency} \\`);
  lines.push('  trace.txt');

  return lines.join('\n');
}
