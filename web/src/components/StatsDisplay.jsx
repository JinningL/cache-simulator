import './StatsDisplay.css';

/* Ordered list of possible level keys so we render them top-to-bottom */
const LEVEL_ORDER = ['L1', 'L2', 'L3'];

function HitBar({ rate, colorClass }) {
  return (
    <div className="hit-bar-container" title={`${rate.toFixed(1)}% hit rate`}>
      <div className={`hit-bar-fill ${colorClass}`} style={{ width: `${Math.min(rate, 100)}%` }} />
      <span className="hit-bar-label">{rate.toFixed(1)}%</span>
    </div>
  );
}

function LevelCard({ name, stats }) {
  const colorClass = `level-${name.toLowerCase()}`;
  return (
    <div className={`level-card ${colorClass}`}>
      <div className="level-card-header">
        <span className={`level-badge badge-${name.toLowerCase()}`}>{name}</span>
        <HitBar rate={stats.hitRate ?? 0} colorClass={`fill-${name.toLowerCase()}`} />
      </div>
      <table className="stats-table">
        <tbody>
          <tr>
            <td>Accesses</td><td>{(stats.accesses ?? 0).toLocaleString()}</td>
            <td>Reads</td>   <td>{(stats.reads    ?? 0).toLocaleString()}</td>
            <td>Writes</td>  <td>{(stats.writes   ?? 0).toLocaleString()}</td>
          </tr>
          <tr>
            <td>Hits</td>    <td className="val-hit">{(stats.hits   ?? 0).toLocaleString()}</td>
            <td>Misses</td>  <td className="val-miss">{(stats.misses ?? 0).toLocaleString()}</td>
            <td>Miss Rate</td><td>{(stats.missRate ?? 0).toFixed(1)}%</td>
          </tr>
          <tr>
            <td>Evictions</td>  <td>{(stats.evictions       ?? 0).toLocaleString()}</td>
            <td>Dirty WBs</td>  <td>{(stats.dirtyWritebacks ?? 0).toLocaleString()}</td>
            <td></td><td></td>
          </tr>
        </tbody>
      </table>
    </div>
  );
}

export default function StatsDisplay({ levels, overall }) {
  if (!levels || !overall) return null;

  /* Only render levels present in the parsed output */
  const activeLevels = LEVEL_ORDER.filter(k => levels[k]);

  return (
    <div className="stats-display">
      <h2 className="section-title">Cache Statistics</h2>

      <div className="levels-grid">
        {activeLevels.map(name => (
          <LevelCard key={name} name={name} stats={levels[name]} />
        ))}
      </div>

      <div className="overall-card">
        <h3>Overall</h3>
        <div className="overall-row">
          <div className="overall-stat">
            <span className="stat-value">{(overall.totalAccesses  ?? 0).toLocaleString()}</span>
            <span className="stat-label">Total Accesses</span>
          </div>
          <div className="overall-stat">
            <span className="stat-value val-miss">{(overall.memoryAccesses ?? 0).toLocaleString()}</span>
            <span className="stat-label">Memory Accesses</span>
          </div>
          <div className="overall-stat">
            <span className="stat-value val-amat">{(overall.amat ?? 0).toFixed(2)}</span>
            <span className="stat-label">AMAT (cycles)</span>
          </div>
        </div>

        {overall.latencies && (
          <div className="latency-row">
            {overall.latencies.map(({ name, cycles }) => (
              <span key={name} className="latency-chip">
                <span className={`latency-dot dot-${name.toLowerCase()}`} />
                {name} = {cycles} cyc
              </span>
            ))}
            <span className="latency-chip">
              <span className="latency-dot dot-mem" />
              Mem = {overall.memLatency} cyc
            </span>
          </div>
        )}

        <p className="amat-formula">
          {overall.amtFormula
            ? overall.amtFormula
            : 'AMAT = level_latency + miss_rate × (next_level + … + miss_rate × Mem)'}
        </p>
      </div>
    </div>
  );
}
