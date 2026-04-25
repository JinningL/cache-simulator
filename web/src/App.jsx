import { useState } from 'react';
import ConfigPanel from './components/ConfigPanel';
import TraceInput from './components/TraceInput';
import StatsDisplay from './components/StatsDisplay';
import AccessTimeline from './components/AccessTimeline';
import { buildConfig, buildDisplayCommand } from './utils/commandBuilder';
import { validateConfig, validateTrace } from './utils/validation';
import { parseSimulatorOutput } from './utils/outputParser';
import './App.css';

const DEFAULT_FORM = {
  l1Enabled: true,
  l1SizeVal: 32,  l1SizeUnit: 'KB', l1Assoc: '4',  l1Latency: 1,
  l2Enabled: true,
  l2SizeVal: 256, l2SizeUnit: 'KB', l2Assoc: '8',  l2Latency: 10,
  l3Enabled: true,
  l3SizeVal: 8,   l3SizeUnit: 'MB', l3Assoc: '16', l3Latency: 40,
  blockSize: '64',
  policy: 'LRU',
  memLatency: 100,
};

export default function App() {
  const [form, setForm]           = useState(DEFAULT_FORM);
  const [traceText, setTraceText] = useState('');
  const [traceMode, setTraceMode] = useState('paste');
  const [running, setRunning]     = useState(false);
  const [result, setResult]       = useState(null);
  const [configErrors, setConfigErrors] = useState([]);
  const [traceErrors, setTraceErrors]   = useState([]);

  const handleRun = async () => {
    const cfg = buildConfig(form);
    const cErrors = validateConfig(cfg);
    const tErrors = validateTrace(traceText);
    setConfigErrors(cErrors);
    setTraceErrors(tErrors);
    if (cErrors.length || tErrors.length) return;

    setRunning(true);
    setResult(null);
    const displayCmd = buildDisplayCommand(cfg);

    try {
      const resp = await fetch('/api/simulate', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ config: cfg, trace: traceText }),
      });
      const data = await resp.json();

      if (!resp.ok) {
        setResult({ command: displayCmd, error: data.error || 'Simulation failed' });
      } else {
        const parsed = parseSimulatorOutput(data.rawOutput);
        setResult({ command: data.command || displayCmd, rawOutput: data.rawOutput, parsed });
      }
    } catch {
      setResult({
        command: displayCmd,
        error: `Could not reach the simulation server.\n\nStart it with:\n  cd web && npm run dev\n\nGenerated command (run manually):\n\n${displayCmd}`,
      });
    } finally {
      setRunning(false);
    }
  };

  const hasErrors = configErrors.length > 0 || traceErrors.length > 0;

  return (
    <div className="app">
      <header className="app-header">
        <div className="header-inner">
          <div className="header-title">
            <span className="header-icon">⚡</span>
            Cache Simulator
          </div>
          <span className="header-sub">
            Dynamic hierarchy &nbsp;·&nbsp; Write-back + Write-allocate &nbsp;·&nbsp; LRU / FIFO
          </span>
        </div>
      </header>

      <main className="app-body">
        <aside className="left-panel">
          <ConfigPanel form={form} onChange={setForm} />

          {configErrors.length > 0 && (
            <ul className="error-list">
              {configErrors.map((e, i) => <li key={i}>{e}</li>)}
            </ul>
          )}

          <TraceInput
            mode={traceMode}
            traceText={traceText}
            onChange={setTraceText}
            onModeChange={setTraceMode}
            errors={traceErrors}
          />

          <button
            className={`run-btn${running ? ' running' : ''}${hasErrors ? ' disabled' : ''}`}
            onClick={handleRun}
            disabled={running}
            type="button"
          >
            {running ? <><span className="btn-spinner" /> Running…</> : 'Run Simulation'}
          </button>
        </aside>

        <section className="right-panel">
          {!result && !running && (
            <div className="empty-state">
              <div className="empty-icon">🖥️</div>
              <p>Configure the cache hierarchy, paste a memory trace,<br />then click <strong>Run Simulation</strong>.</p>
            </div>
          )}

          {running && (
            <div className="empty-state">
              <div className="spinner-large" />
              <p>Running simulation…</p>
            </div>
          )}

          {result && (
            <div className="results">
              <div className="command-card">
                <div className="command-label">Equivalent CLI command</div>
                <pre className="command-pre">{result.command}</pre>
              </div>

              {result.error ? (
                <div className="error-card">
                  <strong>Error</strong>
                  <pre>{result.error}</pre>
                </div>
              ) : (
                <>
                  <StatsDisplay levels={result.parsed.levels} overall={result.parsed.overall} />
                  <AccessTimeline accesses={result.parsed.accesses} />
                  <details className="raw-output">
                    <summary>Raw simulator output</summary>
                    <pre>{result.rawOutput}</pre>
                  </details>
                </>
              )}
            </div>
          )}
        </section>
      </main>
    </div>
  );
}
