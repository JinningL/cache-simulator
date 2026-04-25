function isPowerOf2(n) {
  return n > 0 && (n & (n - 1)) === 0;
}

export function validateConfig(cfg) {
  const errors = [];
  const { l1Enabled, l1Size, l1Assoc,
          l2Enabled, l2Size, l2Assoc,
          l3Enabled, l3Size, l3Assoc,
          blockSize, l1Latency, l2Latency, l3Latency, memLatency } = cfg;

  /* At least one level must be enabled */
  if (!l1Enabled && !l2Enabled && !l3Enabled)
    errors.push('At least one cache level must be enabled');

  if (!blockSize || !isPowerOf2(blockSize))
    errors.push('Block size must be a power of 2 (e.g. 16, 32, 64, 128 …)');

  const checkLevel = (name, enabled, size, assoc) => {
    if (!enabled) return;
    if (!size || size <= 0)
      errors.push(`${name} size must be positive`);
    else if (blockSize && size % blockSize !== 0)
      errors.push(`${name} size (${size} B) must be divisible by block size (${blockSize} B)`);
    else if (blockSize && assoc) {
      const lines = size / blockSize;
      if (!Number.isInteger(lines / assoc))
        errors.push(`${name}: num_lines (${lines}) must be divisible by associativity (${assoc})`);
    }
  };

  checkLevel('L1', l1Enabled, l1Size, l1Assoc);
  checkLevel('L2', l2Enabled, l2Size, l2Assoc);
  checkLevel('L3', l3Enabled, l3Size, l3Assoc);

  const checkLatency = (name, val) => {
    if (!Number.isInteger(val) || val < 1)
      errors.push(`${name} latency must be a positive integer`);
  };

  if (l1Enabled) checkLatency('L1', l1Latency);
  if (l2Enabled) checkLatency('L2', l2Latency);
  if (l3Enabled) checkLatency('L3', l3Latency);
  checkLatency('Memory', memLatency);

  return errors;
}

export function validateTrace(traceText) {
  if (!traceText || !traceText.trim())
    return ['Trace is empty — add at least one R or W line'];

  const errors = [];
  const lines = traceText.split('\n');
  let hasEntry = false;

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim();
    if (!line || line.startsWith('#')) continue;
    hasEntry = true;

    if (!/^[RW]\s+0x[0-9a-fA-F]+$/.test(line))
      errors.push(`Line ${i + 1}: invalid — got "${line}". Expected: R 0x1234 or W 0xABCD`);

    if (errors.length >= 5) {
      errors.push('… more errors omitted. Fix the above first.');
      break;
    }
  }

  if (!hasEntry)
    errors.push('Trace has no valid entries (only blank lines / comments)');

  return errors;
}
