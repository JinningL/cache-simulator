import { useRef } from 'react';
import './TraceInput.css';

const SAMPLE_TRACE = `# Sample memory trace
R 0x00001A3C
W 0x00001A40
R 0x00001A44
W 0x00002000
R 0x00003FFC
R 0x00001A3C
R 0x00001A40
W 0x00004000
R 0x00005000
W 0x00006000`;

export default function TraceInput({ mode, traceText, onChange, onModeChange, errors }) {
  const fileRef = useRef(null);

  const handleFile = (e) => {
    const file = e.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (ev) => onChange(ev.target.result);
    reader.readAsText(file);
  };

  const loadSample = () => {
    onModeChange('paste');
    onChange(SAMPLE_TRACE);
  };

  return (
    <div className="trace-input">
      <div className="trace-header">
        <h2>Memory Trace</h2>
        <button className="sample-btn" type="button" onClick={loadSample}>
          Load Sample
        </button>
      </div>

      <div className="trace-tabs">
        {['paste', 'upload'].map(m => (
          <button
            key={m}
            className={`tab ${mode === m ? 'active' : ''}`}
            type="button"
            onClick={() => onModeChange(m)}
          >
            {m === 'paste' ? 'Paste Text' : 'Upload File'}
          </button>
        ))}
      </div>

      {mode === 'paste' ? (
        <textarea
          className="trace-textarea"
          placeholder={"# Format: R or W followed by a hex address\nR 0x1A3C\nW 0x2000"}
          value={traceText}
          onChange={e => onChange(e.target.value)}
          spellCheck={false}
        />
      ) : (
        <div className="upload-area" onClick={() => fileRef.current.click()}>
          <input
            ref={fileRef}
            type="file"
            accept=".txt"
            style={{ display: 'none' }}
            onChange={handleFile}
          />
          <div className="upload-icon">📂</div>
          <p>Click to upload <strong>trace.txt</strong></p>
          <p className="upload-hint">Plain text, one access per line</p>
          {traceText && (
            <p className="upload-loaded">
              ✓ {traceText.split('\n').filter(l => l.trim() && !l.trim().startsWith('#')).length} entries loaded
            </p>
          )}
        </div>
      )}

      <div className="trace-format-hint">
        <code>R 0x1A3C</code> — read &nbsp;|&nbsp; <code>W 0x2000</code> — write &nbsp;|&nbsp; lines starting with <code>#</code> are comments
      </div>

      {errors.length > 0 && (
        <ul className="error-list">
          {errors.map((e, i) => <li key={i}>{e}</li>)}
        </ul>
      )}
    </div>
  );
}
