import './AccessTimeline.css';

const RESULT_LABELS = {
  L1:     { label: 'L1 HIT',  cls: 'badge-l1'  },
  L2:     { label: 'L2 HIT',  cls: 'badge-l2'  },
  L3:     { label: 'L3 HIT',  cls: 'badge-l3'  },
  MEMORY: { label: 'MEMORY',  cls: 'badge-mem' },
};

function AccessRow({ access, idx }) {
  const badge = RESULT_LABELS[access.result] ?? RESULT_LABELS.MEMORY;
  return (
    <tr className={`access-row result-${access.result.toLowerCase()}`}>
      <td className="col-idx">{idx + 1}</td>
      <td className={`col-op op-${access.op}`}>{access.op}</td>
      <td className="col-addr">{access.addr}</td>
      <td className="col-badge">
        <span className={`badge ${badge.cls}`}>{badge.label}</span>
      </td>
      <td className="col-detail">{access.detail}</td>
    </tr>
  );
}

export default function AccessTimeline({ accesses }) {
  if (!accesses || accesses.length === 0) return null;

  /* Summary counts */
  const counts = accesses.reduce((acc, a) => {
    acc[a.result] = (acc[a.result] || 0) + 1;
    return acc;
  }, {});

  return (
    <div className="access-timeline">
      <div className="timeline-header">
        <h2 className="section-title">Access Log</h2>
        <div className="summary-badges">
          {Object.entries(counts).map(([level, count]) => {
            const b = RESULT_LABELS[level] ?? RESULT_LABELS.MEMORY;
            return (
              <span key={level} className={`badge ${b.cls}`}>
                {b.label}: {count}
              </span>
            );
          })}
          <span className="badge badge-total">Total: {accesses.length}</span>
        </div>
      </div>

      <div className="timeline-scroll">
        <table className="timeline-table">
          <thead>
            <tr>
              <th>#</th>
              <th>Op</th>
              <th>Address</th>
              <th>Result</th>
              <th>Detail</th>
            </tr>
          </thead>
          <tbody>
            {accesses.map((a, i) => (
              <AccessRow key={i} access={a} idx={i} />
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}
