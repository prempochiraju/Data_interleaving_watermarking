# Watermark Log Analysis

Use this folder to process transmitter and receiver serial logs and generate proof plots for the DITMC payload-level watermarking experiment.

## Inputs

Use logs captured with PlatformIO:

```powershell
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe device monitor -b 115200 -p COM_PORT --filter log2file
```

Capture both sides:

- Transmitter log from transmitter laptop
- Receiver log from receiver laptop

For matched results, use logs after the sync messages:

```text
Experiment sync received. Receiver counters reset for matched TX/RX logging.
Transmitter counters reset after sync; experiment totals start now.
```

## Run

From the project root:

```powershell
C:\Users\pyppr\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe tools\watermark_analysis\analyze_watermark_logs.py `
  --transmitter-log C:\Users\pyppr\Downloads\device-monitor-transmitter.log.txt `
  --receiver-log C:\Users\pyppr\Downloads\device-monitor-Reciver.log.txt `
  --output-dir logs\proof_run
```

If normal Python is installed and on PATH, this also works:

```powershell
python tools\watermark_analysis\analyze_watermark_logs.py --transmitter-log TX.log --receiver-log RX.log --output-dir logs\proof_run
```

## Outputs

The script creates:

- `watermark_proof.svg`: main proof graph
- `validation_pie.svg`: valid vs invalid watermark distribution
- `transmitter_stats.csv`: parsed transmitter statistics
- `receiver_stats.csv`: parsed receiver statistics
- `watermark_analysis_summary.txt`: plain-English proof summary

The script has no third-party package requirements.

## Proof Rule

Watermarking is working when the receiver reports:

```text
Watermark blocks received > 0
Valid watermarks > 0
```

A clean run should also show:

```text
Invalid watermarks = 0
Watermark accuracy = 100.00%
```

If invalid watermarks are present, the watermarking mechanism still ran, but the run was not clean. Check synchronized logging, receiver MAC address, channel, board distance, and packet corruption conditions.
