#!/usr/bin/env python3
"""Turn Google Benchmark JSON files into a standalone interactive report."""

import argparse
import json
from pathlib import Path
from statistics import median


TIME_TO_NS = {"ns": 1, "us": 1_000, "ms": 1_000_000, "s": 1_000_000_000}


def read_results(path: Path) -> list[dict]:
    document = json.loads(path.read_text(encoding="utf-8"))
    grouped: dict[str, list[float]] = {}
    suite = "tree" if path.stem == "tree" else "search"

    for entry in document["benchmarks"]:
        if entry.get("run_type") != "iteration":
            continue
        name = entry["name"]
        grouped.setdefault(name, []).append(
            entry["cpu_time"] * TIME_TO_NS[entry["time_unit"]]
        )

    rows = []
    for name, runs in grouped.items():
        parts = name.split("/")
        if len(parts) != 3 or parts[1] not in {"hit", "miss"}:
            continue
        rows.append({
            "build": path.parent.name,
            "suite": suite,
            "engine": parts[0],
            "workload": parts[1],
            "size": int(parts[2]),
            "median_ns": median(runs),
            "runs_ns": runs,
        })
    return rows


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("results", nargs="+", type=Path, help="Google Benchmark JSON files")
    parser.add_argument("-o", "--output", type=Path, default=Path("benchmark-report.html"))
    args = parser.parse_args()

    rows = [row for path in args.results for row in read_results(path)]
    if not rows:
        parser.error("no supported benchmark results found")

    data = json.dumps(rows).replace("<", "\\u003c")
    html = HTML.replace("__DATA__", data)
    args.output.write_text(html, encoding="utf-8")
    print(f"Wrote {args.output} ({len(rows)} cases)")


HTML = """<!doctype html>
<html lang="en">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>B-tree benchmark report</title>
<style>
  body { font: 16px system-ui, sans-serif; margin: 0; color: #18212f; background: #f6f8fb; }
  main { max-width: 1500px; margin: auto; padding: 32px 20px; }
  h1 { margin-bottom: 6px; }
  p { line-height: 1.5; color: #445064; }
  nav, .controls { display: flex; gap: 10px; flex-wrap: wrap; margin: 24px 0; }
  button, select { font: inherit; padding: 8px 12px; border: 1px solid #bbc7d8; border-radius: 8px; background: white; }
  button { cursor: pointer; }
  button.active { background: #244da8; color: white; border-color: #244da8; }
  label { display: grid; gap: 5px; font-size: 14px; }
  section { background: white; border-radius: 12px; padding: 22px; margin: 18px 0; box-shadow: 0 1px 5px #dce2eb; }
  .hidden { display: none; }
  .table-wrap { overflow-x: auto; }
  table { border-collapse: collapse; width: 100%; }
  th, td { text-align: left; padding: 9px; border-bottom: 1px solid #e5eaf1; }
  th { background: #f0f4fa; }
  #plot { width: 100%; height: min(65vw, 650px); min-height: 460px; }
  .legend { display: grid; grid-template-columns: repeat(auto-fit, minmax(210px, 1fr)); gap: 8px; }
  .legend button { display: flex; align-items: center; gap: 8px; font-size: 14px; }
  .legend button.active { background: #e7edf8; color: #18212f; border-color: #7890bd; }
  .legend svg { width: 42px; height: 16px; flex: none; }
</style>
<main>
  <h1>B-tree benchmark report</h1>
  <p>Median CPU nanoseconds per lookup. Lower is faster.</p>
  <nav>
    <button class="active" data-tab="chart">Chart</button>
    <button data-tab="table">Table</button>
    <button data-tab="about">How to read</button>
  </nav>
  <div class="controls">
    <label>Suite<select id="suite"><option value="search">Node search</option><option value="tree">Whole-tree lookup</option></select></label>
    <label>Build<select id="build"></select></label>
    <label>Workload<select id="workload"><option>hit</option><option>miss</option></select></label>
  </div>
  <section id="chart"><h2>Time by key count</h2><p>Click a series label to isolate it; use “Show all” to restore the chart.</p><svg id="plot" role="img" aria-label="Lookup time by key count"></svg><div class="legend" id="legend"></div></section>
  <section id="table" class="hidden"><h2>Measurements</h2><div class="table-wrap"><table><thead><tr><th>Engine</th><th>Keys</th><th>Median ns</th><th>Runs</th></tr></thead><tbody id="rows"></tbody></table></div></section>
  <section id="about" class="hidden"><h2>How to read this report</h2><p>Each point is the median CPU time from repeated runs. The table shows every measured run. Compare containers or engines at the same key count, workload, and build. Node search and whole-tree lookup are separate suites; their times measure different operations. Build flags, CPU conditions, and query distribution affect results.</p></section>
</main>
<script>
const data = __DATA__;
const colors = ['#2454b5', '#c65327', '#19836d', '#8b45a1', '#b58320', '#555'];
const markers = ['circle', 'square', 'triangle', 'diamond', 'cross', 'plus'];
const dashes = ['', '7 4', '2 3', '9 3 2 3'];
const build = document.querySelector('#build');
const suite = document.querySelector('#suite');
const workload = document.querySelector('#workload');
if (!data.some(row => row.suite === suite.value)) suite.value = data[0].suite;
document.querySelectorAll('nav button').forEach(button => {
  button.addEventListener('click', () => {
    document.querySelectorAll('nav button').forEach(item => item.classList.toggle('active', item === button));
    document.querySelectorAll('main section').forEach(section => section.classList.toggle('hidden', section.id !== button.dataset.tab));
  });
});
function render() {
  const builds = [...new Set(data.filter(row => row.suite === suite.value).map(row => row.build))];
  const selectedBuild = build.value;
  build.replaceChildren(...builds.map(name => new Option(name, name)));
  build.value = builds.includes(selectedBuild) ? selectedBuild : builds[0];
  const rows = data.filter(row => row.suite === suite.value && row.build === build.value && row.workload === workload.value);
  rows.sort((a, b) => a.size - b.size || a.engine.localeCompare(b.engine));
  document.querySelector('#rows').innerHTML = rows.map(row =>
    `<tr><td>${row.engine}</td><td>${row.size}</td><td>${row.median_ns.toFixed(2)}</td><td>${row.runs_ns.map(n => n.toFixed(2)).join(', ')}</td></tr>`
  ).join('');

  const sizes = [...new Set(rows.map(row => row.size))].sort((a, b) => a - b);
  const engines = [...new Set(rows.map(row => row.engine))];
  const max = Math.max(...rows.map(row => row.median_ns), 1);
  const x = size => 110 + sizes.indexOf(size) * 1030 / Math.max(sizes.length - 1, 1);
  const y = value => 535 - value / max * 475;
  const svg = ['<line x1="110" y1="535" x2="1150" y2="535" stroke="#8a96a8"/>',
               '<line x1="110" y1="60" x2="110" y2="535" stroke="#8a96a8"/>'];
  sizes.forEach(size => svg.push(`<text x="${x(size)}" y="565" text-anchor="middle" font-size="18">${size}</text>`));
  [0, .25, .5, .75, 1].forEach(fraction => svg.push(`<text x="98" y="${y(max * fraction) + 6}" text-anchor="end" font-size="16">${(max * fraction).toFixed(1)}</text>`));
  engines.forEach((engine, index) => {
    const points = rows.filter(row => row.engine === engine);
    const color = colors[index % colors.length];
    const dash = dashes[Math.floor(index / colors.length) % dashes.length];
    svg.push(`<polyline data-engine="${engine}" fill="none" stroke="${color}" stroke-width="4" stroke-dasharray="${dash}" points="${points.map(row => `${x(row.size)},${y(row.median_ns)}`).join(' ')}"/>`);
    points.forEach(row => {
      const cx = x(row.size);
      const cy = y(row.median_ns);
      const marker = markers[index % markers.length];
      let shape;
      if (marker === 'circle') shape = `<circle cx="${cx}" cy="${cy}" r="8" fill="${color}"/>`;
      if (marker === 'square') shape = `<rect x="${cx - 7}" y="${cy - 7}" width="14" height="14" fill="${color}"/>`;
      if (marker === 'triangle') shape = `<path d="M ${cx} ${cy - 9} L ${cx + 8} ${cy + 7} L ${cx - 8} ${cy + 7} Z" fill="${color}"/>`;
      if (marker === 'diamond') shape = `<path d="M ${cx} ${cy - 9} L ${cx + 8} ${cy} L ${cx} ${cy + 9} L ${cx - 8} ${cy} Z" fill="${color}"/>`;
      if (marker === 'cross') shape = `<path d="M ${cx - 7} ${cy - 7} L ${cx + 7} ${cy + 7} M ${cx - 7} ${cy + 7} L ${cx + 7} ${cy - 7}" stroke="${color}" stroke-width="4"/>`;
      if (marker === 'plus') shape = `<path d="M ${cx - 8} ${cy} L ${cx + 8} ${cy} M ${cx} ${cy - 8} L ${cx} ${cy + 8}" stroke="${color}" stroke-width="4"/>`;
      svg.push(`<g data-engine="${engine}">${shape}<title>${engine}: ${row.median_ns.toFixed(2)} ns at ${row.size} keys</title></g>`);
    });
  });
  document.querySelector('#plot').setAttribute('viewBox', '0 0 1200 600');
  document.querySelector('#plot').innerHTML = svg.join('');
  document.querySelector('#legend').innerHTML = engines.map((engine, index) => {
    const color = colors[index % colors.length];
    const marker = markers[index % markers.length];
    const dash = dashes[Math.floor(index / colors.length) % dashes.length];
    return `<button data-engine="${engine}"><svg viewBox="0 0 42 16" aria-hidden="true"><line x1="1" y1="8" x2="41" y2="8" stroke="${color}" stroke-width="2.5" stroke-dasharray="${dash}"/><circle cx="21" cy="8" r="4" fill="${color}"/></svg>${engine}</button>`;
  }).join('');
  document.querySelector('#legend').insertAdjacentHTML('afterbegin', '<button id="show-all">Show all</button>');
  document.querySelector('#show-all').addEventListener('click', () => {
    document.querySelectorAll('#legend button').forEach(item => item.classList.remove('active'));
    document.querySelectorAll('#plot [data-engine]').forEach(item => {
      item.style.removeProperty('opacity');
    });
  });
  document.querySelectorAll('#legend button[data-engine]').forEach(button => {
    button.addEventListener('click', () => {
      const selected = button.dataset.engine;
      const isActive = button.classList.contains('active');
      document.querySelectorAll('#legend button').forEach(item => item.classList.toggle('active', !isActive && item === button));
      document.querySelectorAll('#plot [data-engine]').forEach(item => {
        item.style.opacity = isActive || item.dataset.engine === selected ? '1' : '0.12';
      });
    });
  });
}
build.addEventListener('change', render);
suite.addEventListener('change', render);
workload.addEventListener('change', render);
render();
</script>
</html>
"""


if __name__ == "__main__":
    main()
