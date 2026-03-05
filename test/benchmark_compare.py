#!/usr/bin/env python3
"""
UnrealCV Binary 性能对比工具
用于对比新旧编译版本的性能差异

Usage:
    # 先测试旧版本
    python benchmark_compare.py --binary ./UE5_old.exe --tag "baseline" --output baseline.json
    
    # 再测试新版本
    python benchmark_compare.py --binary ./UE5_new.exe --tag "optimized" --output optimized.json
    
    # 对比结果
    python benchmark_compare.py --compare baseline.json optimized.json --report comparison.html
"""

import json
import argparse
import sys
import os
from typing import Dict, List, Any, Optional
from datetime import datetime


def load_report(path: str) -> Dict:
    """加载测试报告"""
    with open(path, 'r') as f:
        return json.load(f)


def compare_reports(baseline: Dict, optimized: Dict) -> Dict[str, Any]:
    """对比两个测试报告"""
    comparison = {
        "timestamp": datetime.now().isoformat(),
        "baseline": {
            "timestamp": baseline['metadata']['timestamp'],
            "total_commands": baseline['summary']['total_commands']
        },
        "optimized": {
            "timestamp": optimized['metadata']['timestamp'],
            "total_commands": optimized['summary']['total_commands']
        },
        "command_comparisons": [],
        "category_summary": {},
        "overall": {}
    }
    
    # 创建命令查找表
    baseline_cmds = {r['cmd']: r for r in baseline['all_results']}
    optimized_cmds = {r['cmd']: r for r in optimized['all_results']}
    
    # 对比每个命令
    all_cmds = set(baseline_cmds.keys()) | set(optimized_cmds.keys())
    
    improvements = []
    regressions = []
    
    for cmd in sorted(all_cmds):
        if cmd not in baseline_cmds:
            comparison['command_comparisons'].append({
                "cmd": cmd,
                "status": "new",
                "optimized": optimized_cmds.get(cmd, {})
            })
            continue
        
        if cmd not in optimized_cmds:
            comparison['command_comparisons'].append({
                "cmd": cmd,
                "status": "removed",
                "baseline": baseline_cmds[cmd]
            })
            continue
        
        b = baseline_cmds[cmd]
        o = optimized_cmds[cmd]
        
        if b.get('avg_latency_ms') and o.get('avg_latency_ms'):
            delta = b['avg_latency_ms'] - o['avg_latency_ms']
            pct_change = (delta / b['avg_latency_ms'] * 100) if b['avg_latency_ms'] > 0 else 0
            
            cmd_cmp = {
                "cmd": cmd,
                "status": "compared",
                "category": b.get('category', 'unknown'),
                "baseline_latency_ms": b['avg_latency_ms'],
                "baseline_fps": b.get('fps', 0),
                "optimized_latency_ms": o['avg_latency_ms'],
                "optimized_fps": o.get('fps', 0),
                "delta_ms": delta,
                "pct_change": pct_change,
                "improved": delta > 0
            }
            
            comparison['command_comparisons'].append(cmd_cmp)
            
            if pct_change > 5:  # 5% 以上提升
                improvements.append(cmd_cmp)
            elif pct_change < -5:  # 5% 以上退化
                regressions.append(cmd_cmp)
    
    # 按类别汇总
    categories = {}
    for cmd_cmp in comparison['command_comparisons']:
        if cmd_cmp['status'] != 'compared':
            continue
        cat = cmd_cmp['category']
        if cat not in categories:
            categories[cat] = {'improvements': 0, 'regressions': 0, 'unchanged': 0, 'total_delta': 0}
        
        if cmd_cmp['improved']:
            categories[cat]['improvements'] += 1
        elif cmd_cmp['pct_change'] < -5:
            categories[cat]['regressions'] += 1
        else:
            categories[cat]['unchanged'] += 1
        
        categories[cat]['total_delta'] += cmd_cmp['delta_ms']
    
    comparison['category_summary'] = categories
    
    # 总体统计
    total_baseline_latency = sum(b['avg_latency_ms'] for b in baseline_cmds.values() if 'avg_latency_ms' in b)
    total_optimized_latency = sum(o['avg_latency_ms'] for o in optimized_cmds.values() if 'avg_latency_ms' in o)
    
    comparison['overall'] = {
        "total_commands_compared": len([c for c in comparison['command_comparisons'] if c['status'] == 'compared']),
        "improvements": len(improvements),
        "regressions": len(regressions),
        "avg_improvement_pct": sum(i['pct_change'] for i in improvements) / len(improvements) if improvements else 0,
        "avg_regression_pct": sum(r['pct_change'] for r in regressions) / len(regressions) if regressions else 0,
        "total_latency_change_ms": total_baseline_latency - total_optimized_latency,
        "total_latency_change_pct": ((total_baseline_latency - total_optimized_latency) / total_baseline_latency * 100) if total_baseline_latency > 0 else 0
    }
    
    comparison['top_improvements'] = sorted(improvements, key=lambda x: x['pct_change'], reverse=True)[:10]
    comparison['top_regressions'] = sorted(regressions, key=lambda x: x['pct_change'])[:10]
    
    return comparison


def generate_html_report(comparison: Dict, output_path: str):
    """生成 HTML 对比报告"""
    html = f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>UnrealCV Binary Performance Comparison</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }}
        .container {{ max-width: 1200px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; }}
        h1 {{ color: #333; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }}
        h2 {{ color: #555; margin-top: 30px; }}
        .summary {{ background: #e8f5e9; padding: 15px; border-radius: 5px; margin: 20px 0; }}
        .improvement {{ color: #4CAF50; }}
        .regression {{ color: #f44336; }}
        .neutral {{ color: #757575; }}
        table {{ width: 100%; border-collapse: collapse; margin: 20px 0; }}
        th {{ background: #4CAF50; color: white; padding: 12px; text-align: left; }}
        td {{ padding: 10px; border-bottom: 1px solid #ddd; }}
        tr:hover {{ background: #f5f5f5; }}
        .metric {{ display: inline-block; margin: 10px 20px 10px 0; padding: 10px; background: #f0f0f0; border-radius: 5px; }}
        .metric-value {{ font-size: 24px; font-weight: bold; color: #4CAF50; }}
        .metric-label {{ font-size: 12px; color: #666; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 UnrealCV Binary Performance Comparison</h1>
        <p>Generated: {comparison['timestamp']}</p>
        
        <div class="summary">
            <h2>Overall Summary</h2>
            <div class="metric">
                <div class="metric-value {('improvement' if comparison['overall']['total_latency_change_pct'] > 0 else 'regression')}">
                    {comparison['overall']['total_latency_change_pct']:.1f}%
                </div>
                <div class="metric-label">Overall Improvement</div>
            </div>
            <div class="metric">
                <div class="metric-value improvement">{comparison['overall']['improvements']}</div>
                <div class="metric-label">Improved Commands</div>
            </div>
            <div class="metric">
                <div class="metric-value regression">{comparison['overall']['regressions']}</div>
                <div class="metric-label">Regressed Commands</div>
            </div>
            <div class="metric">
                <div class="metric-value">{comparison['overall']['total_commands_compared']}</div>
                <div class="metric-label">Commands Compared</div>
            </div>
        </div>
        
        <h2>📊 Category Performance</h2>
        <table>
            <tr>
                <th>Category</th>
                <th>Improvements</th>
                <th>Regressions</th>
                <th>Unchanged</th>
                <th>Total Delta (ms)</th>
            </tr>
"""
    
    for cat, stats in comparison['category_summary'].items():
        html += f"""
            <tr>
                <td>{cat}</td>
                <td class="improvement">{stats['improvements']}</td>
                <td class="regression">{stats['regressions']}</td>
                <td class="neutral">{stats['unchanged']}</td>
                <td>{stats['total_delta']:.2f}</td>
            </tr>
"""
    
    html += """
        </table>
        
        <h2>🚀 Top 10 Improvements</h2>
        <table>
            <tr>
                <th>Command</th>
                <th>Category</th>
                <th>Baseline (ms)</th>
                <th>Optimized (ms)</th>
                <th>Improvement</th>
            </tr>
"""
    
    for imp in comparison.get('top_improvements', []):
        html += f"""
            <tr>
                <td>{imp['cmd']}</td>
                <td>{imp['category']}</td>
                <td>{imp['baseline_latency_ms']:.2f}</td>
                <td>{imp['optimized_latency_ms']:.2f}</td>
                <td class="improvement">+{imp['pct_change']:.1f}%</td>
            </tr>
"""
    
    html += """
        </table>
        
        <h2>⚠️ Top 10 Regressions</h2>
        <table>
            <tr>
                <th>Command</th>
                <th>Category</th>
                <th>Baseline (ms)</th>
                <th>Optimized (ms)</th>
                <th>Regression</th>
            </tr>
"""
    
    for reg in comparison.get('top_regressions', []):
        html += f"""
            <tr>
                <td>{reg['cmd']}</td>
                <td>{reg['category']}</td>
                <td>{reg['baseline_latency_ms']:.2f}</td>
                <td>{reg['optimized_latency_ms']:.2f}</td>
                <td class="regression">{reg['pct_change']:.1f}%</td>
            </tr>
"""
    
    html += """
        </table>
        
        <h2>📋 All Command Comparisons</h2>
        <table>
            <tr>
                <th>Command</th>
                <th>Category</th>
                <th>Baseline</th>
                <th>Optimized</th>
                <th>Delta</th>
                <th>Change</th>
            </tr>
"""
    
    for cmd in comparison['command_comparisons']:
        if cmd['status'] != 'compared':
            continue
        change_class = 'improvement' if cmd['improved'] else ('regression' if cmd['pct_change'] < -5 else 'neutral')
        html += f"""
            <tr>
                <td>{cmd['cmd']}</td>
                <td>{cmd['category']}</td>
                <td>{cmd['baseline_latency_ms']:.2f} ms ({cmd['baseline_fps']:.1f} FPS)</td>
                <td>{cmd['optimized_latency_ms']:.2f} ms ({cmd['optimized_fps']:.1f} FPS)</td>
                <td>{cmd['delta_ms']:.2f} ms</td>
                <td class="{change_class}">{cmd['pct_change']:+.1f}%</td>
            </tr>
"""
    
    html += """
        </table>
    </div>
</body>
</html>
"""
    
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"HTML report saved to: {output_path}")


def generate_text_report(comparison: Dict):
    """生成文本对比报告"""
    print("\n" + "="*70)
    print("UnrealCV Binary Performance Comparison")
    print("="*70)
    print(f"\nTimestamp: {comparison['timestamp']}")
    print(f"Baseline: {comparison['baseline']['timestamp']}")
    print(f"Optimized: {comparison['optimized']['timestamp']}")
    
    overall = comparison['overall']
    print(f"\n{'Overall Summary':=^70}")
    print(f"  Commands Compared: {overall['total_commands_compared']}")
    print(f"  Improvements: {overall['improvements']} (avg {overall['avg_improvement_pct']:.1f}%)")
    print(f"  Regressions: {overall['regressions']} (avg {overall['avg_regression_pct']:.1f}%)")
    print(f"  Total Latency Change: {overall['total_latency_change_pct']:+.1f}%")
    
    print(f"\n{'Category Summary':=^70}")
    for cat, stats in comparison['category_summary'].items():
        print(f"  {cat:20s}: +{stats['improvements']}/-{stats['regressions']} (Δ {stats['total_delta']:+.2f}ms)")
    
    if comparison.get('top_improvements'):
        print(f"\n{'Top 10 Improvements':=^70}")
        for i, imp in enumerate(comparison['top_improvements'], 1):
            print(f"  {i:2d}. {imp['cmd']:50s} {imp['pct_change']:+6.1f}% "
                  f"({imp['baseline_latency_ms']:.1f} → {imp['optimized_latency_ms']:.1f} ms)")
    
    if comparison.get('top_regressions'):
        print(f"\n{'Top 10 Regressions':=^70}")
        for i, reg in enumerate(comparison['top_regressions'], 1):
            print(f"  {i:2d}. {reg['cmd']:50s} {reg['pct_change']:+6.1f}% "
                  f"({reg['baseline_latency_ms']:.1f} → {reg['optimized_latency_ms']:.1f} ms)")
    
    print("\n" + "="*70)


def main():
    parser = argparse.ArgumentParser(description='UnrealCV Binary Performance Comparison')
    parser.add_argument('--baseline', help='Baseline benchmark report (JSON)')
    parser.add_argument('--optimized', help='Optimized benchmark report (JSON)')
    parser.add_argument('--compare', nargs=2, metavar=('BASELINE', 'OPTIMIZED'),
                        help='Compare two existing reports')
    parser.add_argument('--output', default='comparison.json',
                        help='Output comparison JSON file')
    parser.add_argument('--html', help='Generate HTML report')
    parser.add_argument('--text', action='store_true',
                        help='Print text report to stdout')
    
    args = parser.parse_args()
    
    if args.compare:
        # 对比模式
        baseline = load_report(args.compare[0])
        optimized = load_report(args.compare[1])
        comparison = compare_reports(baseline, optimized)
        
        # 保存对比结果
        with open(args.output, 'w') as f:
            json.dump(comparison, f, indent=2)
        print(f"Comparison saved to: {args.output}")
        
        # 生成报告
        if args.text:
            generate_text_report(comparison)
        
        if args.html:
            generate_html_report(comparison, args.html)
    
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == '__main__':
    main()
