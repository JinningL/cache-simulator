import './ConfigPanel.css';

const SIZE_UNITS = ['Bytes', 'KB', 'MB'];
const ASSOC_OPTIONS = [
  { value: 'direct', label: 'Direct-mapped (1-way)' },
  { value: '2',      label: '2-way' },
  { value: '4',      label: '4-way' },
  { value: '8',      label: '8-way' },
  { value: '16',     label: '16-way' },
  { value: 'full',   label: 'Fully Associative' },
];
const BLOCK_SIZES = [8, 16, 32, 64, 128, 256, 512];

function Tooltip({ text }) {
  return <span className="tooltip" title={text}>?</span>;
}

/*
 * One complete cache level section: enable toggle, size, associativity, latency.
 * When disabled the body is collapsed and greyed out.
 */
function LevelSection({ id, label, colorClass, enabled, onToggle,
                        sizeVal, sizeUnit, onSizeVal, onSizeUnit,
                        assoc, onAssoc, latency, onLatency }) {
  return (
    <section className={`level-section${enabled ? '' : ' level-disabled'}`}>
      <div className="level-header">
        <h3 className={`level-label ${colorClass}`}>{label}</h3>
        <label className="enable-toggle">
          <input
            type="checkbox"
            checked={enabled}
            onChange={e => onToggle(e.target.checked)}
          />
          <span className="toggle-track">
            <span className="toggle-thumb" />
          </span>
          <span className="toggle-text">{enabled ? 'Enabled' : 'Disabled'}</span>
        </label>
      </div>

      {enabled && (
        <div className="level-body">
          {/* Size */}
          <div className="field">
            <label>
              Size <Tooltip text="Total capacity of this cache level." />
            </label>
            <div className="size-input">
              <input type="number" min="1" value={sizeVal}
                     onChange={e => onSizeVal(e.target.value)} />
              <select value={sizeUnit} onChange={e => onSizeUnit(e.target.value)}>
                {SIZE_UNITS.map(u => <option key={u}>{u}</option>)}
              </select>
            </div>
          </div>

          {/* Associativity */}
          <div className="field">
            <label>
              Associativity <Tooltip text="Lines per set. More ways = fewer conflicts. 'Fully Associative' sets assoc = num_lines." />
            </label>
            <select value={assoc} onChange={e => onAssoc(e.target.value)}>
              {ASSOC_OPTIONS.map(o => (
                <option key={o.value} value={o.value}>{o.label}</option>
              ))}
            </select>
          </div>

          {/* Latency */}
          <div className="field field-row">
            <label>
              Latency (cycles) <Tooltip text="Hit latency for this level, used in the AMAT formula." />
            </label>
            <input
              type="number" min="1" className="latency-input"
              value={latency}
              onChange={e => onLatency(e.target.value)}
            />
          </div>
        </div>
      )}
    </section>
  );
}

export default function ConfigPanel({ form, onChange }) {
  const set = key => val => onChange({ ...form, [key]: val });

  return (
    <div className="config-panel">
      <h2>Cache Configuration</h2>

      <LevelSection
        id="l1" label="L1 Cache" colorClass="l1"
        enabled={form.l1Enabled}  onToggle={set('l1Enabled')}
        sizeVal={form.l1SizeVal}   onSizeVal={set('l1SizeVal')}
        sizeUnit={form.l1SizeUnit} onSizeUnit={set('l1SizeUnit')}
        assoc={form.l1Assoc}       onAssoc={set('l1Assoc')}
        latency={form.l1Latency}   onLatency={set('l1Latency')}
      />

      <LevelSection
        id="l2" label="L2 Cache" colorClass="l2"
        enabled={form.l2Enabled}  onToggle={set('l2Enabled')}
        sizeVal={form.l2SizeVal}   onSizeVal={set('l2SizeVal')}
        sizeUnit={form.l2SizeUnit} onSizeUnit={set('l2SizeUnit')}
        assoc={form.l2Assoc}       onAssoc={set('l2Assoc')}
        latency={form.l2Latency}   onLatency={set('l2Latency')}
      />

      <LevelSection
        id="l3" label="L3 Cache" colorClass="l3"
        enabled={form.l3Enabled}  onToggle={set('l3Enabled')}
        sizeVal={form.l3SizeVal}   onSizeVal={set('l3SizeVal')}
        sizeUnit={form.l3SizeUnit} onSizeUnit={set('l3SizeUnit')}
        assoc={form.l3Assoc}       onAssoc={set('l3Assoc')}
        latency={form.l3Latency}   onLatency={set('l3Latency')}
      />

      {/* Shared parameters */}
      <section className="level-section">
        <h3 className="level-label shared">Shared Parameters</h3>

        <div className="field">
          <label>
            Block Size <Tooltip text="Bytes per cache line — the transfer unit between levels. Must be a power of 2." />
          </label>
          <select value={form.blockSize} onChange={e => set('blockSize')(e.target.value)}>
            {BLOCK_SIZES.map(b => <option key={b} value={b}>{b} bytes</option>)}
          </select>
        </div>

        <div className="field">
          <label>
            Replacement Policy <Tooltip text="LRU evicts the least recently used line. FIFO evicts the oldest loaded line." />
          </label>
          <div className="policy-toggle">
            {['LRU', 'FIFO'].map(p => (
              <button key={p} type="button"
                className={`policy-btn${form.policy === p ? ' active' : ''}`}
                onClick={() => set('policy')(p)}>
                {p}
              </button>
            ))}
          </div>
        </div>

        <div className="field field-row">
          <label>
            Memory Latency (cycles) <Tooltip text="Penalty for a cache miss that reaches main memory. Used in AMAT calculation." />
          </label>
          <input
            type="number" min="1" className="latency-input"
            value={form.memLatency}
            onChange={e => set('memLatency')(e.target.value)}
          />
        </div>
      </section>
    </div>
  );
}
