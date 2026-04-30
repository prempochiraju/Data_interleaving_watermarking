#!/usr/bin/env python3
"""Parse DITMC transmitter/receiver logs and generate watermark proof plots."""

from __future__ import annotations

import argparse
import csv
import re
from pathlib import Path


TX_SYNC_MARKER = "Transmitter counters reset after sync; experiment totals start now."
RX_SYNC_MARKER = "Experiment sync received. Receiver counters reset for matched TX/RX logging."

PATTERNS = {
    "voice": r"Voice bytes (?:sent|received):\s+(\d+)",
    "data": r"(?:Interleaved data bytes|Data bytes received):\s+(\d+)",
    "wm_bytes": r"Watermark bytes (?:sent|received):\s+(\d+)",
    "wm_sent": r"Watermark blocks sent:\s+(\d+)",
    "wm_recv": r"Watermark blocks received:\s+(\d+)",
    "valid": r"Valid watermarks:\s+(\d+)",
    "invalid": r"Invalid watermarks:\s+(\d+)",
    "accuracy": r"Watermark accuracy:\s+([0-9.]+)%",
    "overhead": r"Watermark overhead:\s+([0-9.]+)%",
    "packets_sent": r"Packets sent:\s+(\d+)",
    "packets_recv": r"Packets received:\s+(\d+)",
    "failures": r"Send failures:\s+(\d+)",
}


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def trim_after_marker(text: str, marker: str) -> tuple[str, bool]:
    index = text.find(marker)
    if index < 0:
        return text, False
    return text[index + len(marker) :], True


def parse_stat_blocks(text: str, kind: str) -> list[dict[str, float | int]]:
    header = f"DITMC STATISTICS ({kind})"
    blocks = text.split(header)[1:]
    rows: list[dict[str, float | int]] = []

    for sample_number, block in enumerate(blocks, 1):
        row: dict[str, float | int] = {"sample": sample_number}
        for key, pattern in PATTERNS.items():
            match = re.search(pattern, block)
            if not match:
                continue

            value = match.group(1)
            row[key] = float(value) if "." in value else int(value)

        rows.append(row)

    return rows


def dedupe_cumulative_rows(rows: list[dict[str, float | int]], keys: list[str]) -> list[dict[str, float | int]]:
    """Remove repeated idle statistics where counters did not change."""
    output: list[dict[str, float | int]] = []
    seen: set[tuple[float | int | None, ...]] = set()

    for row in rows:
        signature = tuple(row.get(key) for key in keys)
        if signature in seen:
            continue
        seen.add(signature)
        output.append(row)

    for sample_number, row in enumerate(output, 1):
        row["sample"] = sample_number

    return output


def write_csv(path: Path, rows: list[dict[str, float | int]]) -> None:
    keys = sorted({key for row in rows for key in row})
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=keys)
        writer.writeheader()
        writer.writerows(rows)


def latest(rows: list[dict[str, float | int]]) -> dict[str, float | int]:
    return rows[-1] if rows else {}


def value(row: dict[str, float | int], key: str, default: float | int = 0) -> float | int:
    return row.get(key, default)


def plot_results(
    output_dir: Path,
    tx_rows: list[dict[str, float | int]],
    rx_rows: list[dict[str, float | int]],
) -> None:
    width, height = 1200, 820
    panel_w, panel_h = 520, 300
    panels = [(60, 80), (660, 80), (60, 460), (660, 460)]

    def polyline(rows: list[dict[str, float | int]], key: str, panel: tuple[int, int], y_max: float | None = None) -> str:
        if not rows:
            return ""

        x0, y0 = panel
        values = [float(value(row, key)) for row in rows]
        max_x = max(1, len(rows) - 1)
        max_y = y_max if y_max is not None else max(values + [1.0])
        points = []
        for index, val in enumerate(values):
            x = x0 + 55 + (panel_w - 85) * index / max_x
            y = y0 + panel_h - 45 - (panel_h - 85) * val / max_y
            points.append(f"{x:.1f},{y:.1f}")
        return " ".join(points)

    def panel(title: str, panel_xy: tuple[int, int], y_label: str) -> str:
        x0, y0 = panel_xy
        return f"""
<rect x="{x0}" y="{y0}" width="{panel_w}" height="{panel_h}" fill="#ffffff" stroke="#c9ced6"/>
<text x="{x0 + 18}" y="{y0 + 28}" font-size="18" font-weight="700">{title}</text>
<text x="{x0 + 18}" y="{y0 + panel_h - 14}" font-size="12">Stats sample</text>
<text x="{x0 + 18}" y="{y0 + 55}" font-size="12">{y_label}</text>
<line x1="{x0 + 55}" y1="{y0 + panel_h - 45}" x2="{x0 + panel_w - 30}" y2="{y0 + panel_h - 45}" stroke="#77808f"/>
<line x1="{x0 + 55}" y1="{y0 + 45}" x2="{x0 + 55}" y2="{y0 + panel_h - 45}" stroke="#77808f"/>
"""

    max_blocks = max(
        [float(value(row, "wm_sent")) for row in tx_rows] +
        [float(value(row, "wm_recv")) for row in rx_rows] +
        [1.0]
    )
    max_valid = max(
        [float(value(row, "valid")) for row in rx_rows] +
        [float(value(row, "invalid")) for row in rx_rows] +
        [1.0]
    )
    max_overhead = max(
        [float(value(row, "overhead")) for row in tx_rows] +
        [float(value(row, "overhead")) for row in rx_rows] +
        [1.0]
    )

    svg = [f"""<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">
<rect width="100%" height="100%" fill="#f6f8fb"/>
<text x="60" y="45" font-size="26" font-weight="800">DITMC Payload Watermarking Proof From Serial Logs</text>
"""]

    svg.append(panel("Watermark Blocks", panels[0], "Cumulative blocks"))
    svg.append(f'<polyline fill="none" stroke="#1f77b4" stroke-width="3" points="{polyline(tx_rows, "wm_sent", panels[0], max_blocks)}"/>')
    svg.append(f'<polyline fill="none" stroke="#ff7f0e" stroke-width="3" points="{polyline(rx_rows, "wm_recv", panels[0], max_blocks)}"/>')
    svg.append(f'<text x="{panels[0][0] + 280}" y="{panels[0][1] + 58}" font-size="13" fill="#1f77b4">TX sent</text>')
    svg.append(f'<text x="{panels[0][0] + 360}" y="{panels[0][1] + 58}" font-size="13" fill="#ff7f0e">RX received</text>')

    svg.append(panel("Receiver Validation Counts", panels[1], "Cumulative blocks"))
    svg.append(f'<polyline fill="none" stroke="#2ca02c" stroke-width="3" points="{polyline(rx_rows, "valid", panels[1], max_valid)}"/>')
    svg.append(f'<polyline fill="none" stroke="#d62728" stroke-width="3" points="{polyline(rx_rows, "invalid", panels[1], max_valid)}"/>')
    svg.append(f'<text x="{panels[1][0] + 300}" y="{panels[1][1] + 58}" font-size="13" fill="#2ca02c">Valid</text>')
    svg.append(f'<text x="{panels[1][0] + 360}" y="{panels[1][1] + 58}" font-size="13" fill="#d62728">Invalid</text>')

    svg.append(panel("Receiver Watermark Accuracy", panels[2], "Accuracy (%)"))
    svg.append(f'<polyline fill="none" stroke="#2ca02c" stroke-width="3" points="{polyline(rx_rows, "accuracy", panels[2], 100.0)}"/>')

    svg.append(panel("Watermark Overhead", panels[3], "Overhead (%)"))
    svg.append(f'<polyline fill="none" stroke="#1f77b4" stroke-width="3" points="{polyline(tx_rows, "overhead", panels[3], max_overhead)}"/>')
    svg.append(f'<polyline fill="none" stroke="#ff7f0e" stroke-width="3" points="{polyline(rx_rows, "overhead", panels[3], max_overhead)}"/>')
    svg.append(f'<text x="{panels[3][0] + 300}" y="{panels[3][1] + 58}" font-size="13" fill="#1f77b4">TX</text>')
    svg.append(f'<text x="{panels[3][0] + 350}" y="{panels[3][1] + 58}" font-size="13" fill="#ff7f0e">RX</text>')

    svg.append("</svg>\n")
    (output_dir / "watermark_proof.svg").write_text("\n".join(svg), encoding="utf-8")

    rx_latest = latest(rx_rows)
    valid = int(value(rx_latest, "valid"))
    invalid = int(value(rx_latest, "invalid"))
    total = valid + invalid
    valid_pct = (valid / total * 100) if total else 0.0
    invalid_pct = 100.0 - valid_pct if total else 0.0
    pie_svg = f"""<svg xmlns="http://www.w3.org/2000/svg" width="700" height="360" viewBox="0 0 700 360">
<rect width="100%" height="100%" fill="#f6f8fb"/>
<text x="40" y="45" font-size="24" font-weight="800">Receiver Watermark Validation Result</text>
<rect x="80" y="90" width="{max(1, valid_pct) * 4:.1f}" height="60" fill="#2ca02c"/>
<rect x="80" y="180" width="{max(1, invalid_pct) * 4:.1f}" height="60" fill="#d62728"/>
<text x="80" y="170" font-size="18">Valid watermarks: {valid} ({valid_pct:.2f}%)</text>
<text x="80" y="260" font-size="18">Invalid watermarks: {invalid} ({invalid_pct:.2f}%)</text>
</svg>
"""
    (output_dir / "validation_pie.svg").write_text(pie_svg, encoding="utf-8")


def build_summary(
    tx_rows: list[dict[str, float | int]],
    rx_rows: list[dict[str, float | int]],
    tx_synced: bool,
    rx_synced: bool,
) -> str:
    tx_latest = latest(tx_rows)
    rx_latest = latest(rx_rows)

    tx_sent = int(value(tx_latest, "wm_sent"))
    rx_received = int(value(rx_latest, "wm_recv"))
    valid = int(value(rx_latest, "valid"))
    invalid = int(value(rx_latest, "invalid"))
    accuracy = float(value(rx_latest, "accuracy", (valid / rx_received * 100) if rx_received else 0))

    proof_lines = []
    if rx_received > 0 and valid > 0:
        proof_lines.append("PASS: The receiver detected and validated CRC32 watermark blocks.")
    else:
        proof_lines.append("FAIL: No valid receiver watermark blocks were found.")

    if rx_received > 0 and invalid == 0:
        proof_lines.append("CLEAN RUN: No invalid watermarks were reported.")
    elif rx_received > 0:
        proof_lines.append("NOT CLEAN: Invalid watermarks were reported, so this run is not a 100% validation run.")

    if tx_sent and rx_received:
        difference = rx_received - tx_sent
        proof_lines.append(f"TX/RX block difference: receiver - transmitter = {difference}.")

    return f"""DITMC Watermark Log Analysis

Sync markers:
- Transmitter sync marker found: {tx_synced}
- Receiver sync marker found: {rx_synced}

Transmitter latest:
- Watermark blocks sent: {tx_sent}
- Watermark bytes sent: {value(tx_latest, 'wm_bytes', 'n/a')}
- Send failures: {value(tx_latest, 'failures', 'n/a')}
- Packets sent: {value(tx_latest, 'packets_sent', 'n/a')}
- Watermark overhead: {value(tx_latest, 'overhead', 'n/a')}%

Receiver latest:
- Watermark blocks received: {rx_received}
- Valid watermarks: {valid}
- Invalid watermarks: {invalid}
- Watermark accuracy: {accuracy:.2f}%
- Watermark bytes received: {value(rx_latest, 'wm_bytes', 'n/a')}
- Packets received: {value(rx_latest, 'packets_recv', 'n/a')}
- Watermark overhead: {value(rx_latest, 'overhead', 'n/a')}%

Proof:
{chr(10).join('- ' + line for line in proof_lines)}

How to state the result:
- The watermarking technique is present and working if valid watermarks are greater than zero.
- For a perfect run, invalid watermarks should be zero and accuracy should be 100.00%.
"""


def main() -> None:
    parser = argparse.ArgumentParser(description="Analyze DITMC watermark serial logs.")
    parser.add_argument("--transmitter-log", required=True, type=Path)
    parser.add_argument("--receiver-log", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)

    tx_text, tx_synced = trim_after_marker(read_text(args.transmitter_log), TX_SYNC_MARKER)
    rx_text, rx_synced = trim_after_marker(read_text(args.receiver_log), RX_SYNC_MARKER)

    tx_rows = parse_stat_blocks(tx_text, "Transmitter")
    rx_rows = parse_stat_blocks(rx_text, "Receiver")
    rx_rows = dedupe_cumulative_rows(rx_rows, ["packets_recv", "wm_recv", "valid", "invalid"])

    write_csv(args.output_dir / "transmitter_stats.csv", tx_rows)
    write_csv(args.output_dir / "receiver_stats.csv", rx_rows)

    plot_results(args.output_dir, tx_rows, rx_rows)

    summary = build_summary(tx_rows, rx_rows, tx_synced, rx_synced)
    (args.output_dir / "watermark_analysis_summary.txt").write_text(summary, encoding="utf-8")

    print(summary)
    print("Created:")
    print(args.output_dir / "watermark_proof.svg")
    print(args.output_dir / "validation_pie.svg")
    print(args.output_dir / "transmitter_stats.csv")
    print(args.output_dir / "receiver_stats.csv")
    print(args.output_dir / "watermark_analysis_summary.txt")


if __name__ == "__main__":
    main()
