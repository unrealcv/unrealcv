#!/usr/bin/env python3
"""
Extended Benchmark Suite for UnrealCV Python API
支持扩展测试配置和高级性能测试

Usage:
    python benchmark_extended.py --config benchmark_extended.json --report report.json
    python benchmark_extended.py --suite camera_capture_memory  # 只运行特定测试套件
    python benchmark_extended.py --binary /path/to/UE5.exe --map TestMap
"""

from __future__ import print_function
import json
import time
import argparse
import sys
import os
import statistics
from datetime import datetime
from typing import List, Dict, Any, Optional, Tuple

# 添加 client 路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../client/python'))

from unrealcv import client
from unrealcv.api import UnrealCv_API, MsgDecoder
from unrealcv.launcher import RunUnreal
from unrealcv.util import measure_fps, parse_resolution


class BenchmarkResult:
    """单个命令的测试结果"""
    def __init__(self, cmd: str, category: str):
        self.cmd = cmd
        self.category = category
        self.latencies = []  # 每次调用的延迟(ms)
        self.errors = 0
        self.sample_output = None
        self.start_time = None
        self.end_time = None
    
    def add_latency(self, latency_ms: float):
        self.latencies.append(latency_ms)
    
    def add_error(self):
        self.errors += 1
    
    def summary(self) -> Dict[str, Any]:
        if not self.latencies:
            return {"error": "No data collected"}
        
        return {
            "cmd": self.cmd,
            "category": self.category,
            "count": len(self.latencies),
            "errors": self.errors,
            "total_time_ms": sum(self.latencies),
            "avg_latency_ms": statistics.mean(self.latencies),
            "median_latency_ms": statistics.median(self.latencies),
            "min_latency_ms": min(self.latencies),
            "max_latency_ms": max(self.latencies),
            "std_latency_ms": statistics.stdev(self.latencies) if len(self.latencies) > 1 else 0,
            "fps": 1000.0 / statistics.mean(self.latencies) if self.latencies else 0,
            "sample_output": str(self.sample_output)[:200] if self.sample_output else None
        }


class ExtendedBenchmark:
    """扩展性能测试框架"""
    
    def __init__(self, config_path: str, api: Optional[UnrealCv_API] = None):
        self.config = json.load(open(config_path))
        self.api = api
        self.results = []
        self.first_object = None
        self.test_objects = []
        
    def connect(self, port: int = 9000, ip: str = '127.0.0.1'):
        """连接到 UnrealCV 服务器"""
        if self.api is None:
            client.connect((ip, port))
            # 获取第一个物体用于物体相关测试
            self._prepare_objects()
        return self
    
    def _prepare_objects(self):
        """准备测试用的物体"""
        try:
            objects_str = client.request('vget /objects')
            self.test_objects = objects_str.split() if objects_str else []
            self.first_object = self.test_objects[0] if self.test_objects else None
            print(f"[INFO] Found {len(self.test_objects)} objects, first: {self.first_object}")
        except Exception as e:
            print(f"[WARN] Failed to get objects: {e}")
            self.first_object = "Cube"
    
    def substitute_variables(self, cmd: str) -> str:
        """替换命令中的变量"""
        if '{first_object}' in cmd:
            cmd = cmd.replace('{first_object}', self.first_object or 'Cube')
        return cmd
    
    def run_command(self, cmd: str, timeout: int = 5) -> Tuple[Any, float]:
        """执行单个命令并返回结果和延迟"""
        start = time.perf_counter()
        try:
            if timeout < 0:  # 异步模式
                client.request(cmd, -1)
                result = "async_ok"
            else:
                result = client.request(cmd, timeout)
            latency = (time.perf_counter() - start) * 1000  # ms
            return result, latency
        except Exception as e:
            latency = (time.perf_counter() - start) * 1000
            print(f"[ERROR] Command failed: {cmd}, Error: {e}")
            return None, latency
    
    def run_batch_commands(self, cmds: List[str]) -> Tuple[List[Any], float]:
        """执行批量命令"""
        start = time.perf_counter()
        try:
            results = client.request(cmds)
            latency = (time.perf_counter() - start) * 1000
            return results, latency
        except Exception as e:
            latency = (time.perf_counter() - start) * 1000
            print(f"[ERROR] Batch command failed: {e}")
            return None, latency
    
    def run_count_test(self, task: Dict) -> BenchmarkResult:
        """运行 Count 模式测试(固定次数)"""
        cmd = self.substitute_variables(task['Cmd'])
        count = task.get('Count', 100)
        category = task.get('Category', 'unknown')
        is_async = task.get('Async', False)
        timeout = -1 if is_async else 5
        
        result = BenchmarkResult(cmd, category)
        result.start_time = datetime.now().isoformat()
        
        # Warmup
        for _ in range(self.config['TestConfig'].get('warmup_iterations', 5)):
            self.run_command(cmd, timeout)
        
        # 实际测试
        for i in range(count):
            res, latency = self.run_command(cmd, timeout)
            if res is None:
                result.add_error()
            else:
                result.add_latency(latency)
                if i == 0:
                    result.sample_output = res
        
        result.end_time = datetime.now().isoformat()
        return result
    
    def run_duration_test(self, task: Dict) -> BenchmarkResult:
        """运行 Duration 模式测试(固定时间)"""
        cmd = self.substitute_variables(task['Cmd'])
        duration = task.get('Duration', 5)
        category = task.get('Category', 'unknown')
        
        result = BenchmarkResult(cmd, category)
        result.start_time = datetime.now().isoformat()
        
        # Warmup
        for _ in range(3):
            self.run_command(cmd)
        
        start_time = time.time()
        count = 0
        
        while time.time() - start_time < duration:
            res, latency = self.run_command(cmd)
            count += 1
            if res is None:
                result.add_error()
            else:
                result.add_latency(latency)
                if count == 1:
                    result.sample_output = res
        
        result.end_time = datetime.now().isoformat()
        return result
    
    def run_batch_test(self, task: Dict, batch_cmds: List[str]) -> BenchmarkResult:
        """运行批量命令测试"""
        count = task.get('Count', 50)
        category = task.get('Category', 'batch')
        cmd_str = task['Cmd']
        
        result = BenchmarkResult(cmd_str, category)
        result.start_time = datetime.now().isoformat()
        
        for i in range(count):
            results, latency = self.run_batch_commands(batch_cmds)
            if results is None:
                result.add_error()
            else:
                result.add_latency(latency)
                if i == 0:
                    result.sample_output = str(results)[:200]
        
        result.end_time = datetime.now().isoformat()
        return result
    
    def run_multiview_test(self, task: Dict, viewmodes: List[str]) -> BenchmarkResult:
        """运行多视角图像捕获测试"""
        count = task.get('Count', 20)
        category = task.get('Category', 'multiview')
        cmd_str = task['Cmd']
        
        result = BenchmarkResult(cmd_str, category)
        result.start_time = datetime.now().isoformat()
        
        for i in range(count):
            cmds = [f'vget /camera/0/{vm} bmp' for vm in viewmodes]
            results, latency = self.run_batch_commands(cmds)
            if results is None:
                result.add_error()
            else:
                result.add_latency(latency)
                if i == 0:
                    result.sample_output = f"Captured {len(viewmodes)} views"
        
        result.end_time = datetime.now().isoformat()
        return result
    
    def run_stress_test(self, task: Dict) -> BenchmarkResult:
        """运行压力测试(混合命令)"""
        duration = task.get('Duration', 10)
        category = task.get('Category', 'stress')
        
        stress_cmds = [
            'vget /camera/0/location',
            'vget /camera/0/rotation',
            'vget /camera/0/lit bmp',
            'vget /objects',
        ]
        
        result = BenchmarkResult('stress:mixed', category)
        result.start_time = datetime.now().isoformat()
        
        start_time = time.time()
        cmd_idx = 0
        
        while time.time() - start_time < duration:
            cmd = stress_cmds[cmd_idx % len(stress_cmds)]
            res, latency = self.run_command(cmd)
            if res is None:
                result.add_error()
            else:
                result.add_latency(latency)
            cmd_idx += 1
        
        result.end_time = datetime.now().isoformat()
        return result
    
    def run_suite(self, suite_name: str) -> List[BenchmarkResult]:
        """运行指定的测试套件"""
        if suite_name not in self.config['TestSuites']:
            print(f"[ERROR] Unknown test suite: {suite_name}")
            return []
        
        suite = self.config['TestSuites'][suite_name]
        print(f"\n{'='*60}")
        print(f"Running Test Suite: {suite['name']}")
        print(f"Description: {suite['description']}")
        print(f"{'='*60}")
        
        results = []
        
        # 准备步骤
        if suite.get('prepare') == 'get_first_object' and not self.first_object:
            self._prepare_objects()
        
        for task in suite['tasks']:
            cmd = task['Cmd']
            task_type = task.get('Type', 'single')
            
            print(f"\n[Test] {cmd}")
            
            try:
                if task_type == 'batch':
                    # 批量命令
                    batch_name = cmd.split(':')[1] if ':' in cmd else 'location_rotation'
                    batch_cmds = self.config['BatchTemplates'].get(batch_name, [])
                    batch_cmds = [self.substitute_variables(c) for c in batch_cmds]
                    result = self.run_batch_test(task, batch_cmds)
                
                elif task_type == 'multiview':
                    # 多视角捕获
                    multiview_name = cmd.split(':')[1] if ':' in cmd else 'lit_depth_normal'
                    viewmodes = self.config['MultiviewTemplates'].get(multiview_name, ['lit'])
                    result = self.run_multiview_test(task, viewmodes)
                
                elif task_type == 'stress':
                    # 压力测试
                    result = self.run_stress_test(task)
                
                elif 'Duration' in task:
                    result = self.run_duration_test(task)
                
                elif 'Count' in task:
                    result = self.run_count_test(task)
                
                else:
                    print(f"[SKIP] Unknown task type for {cmd}")
                    continue
                
                results.append(result)
                summary = result.summary()
                print(f"  Count: {summary['count']}, Avg: {summary['avg_latency_ms']:.2f}ms, "
                      f"FPS: {summary['fps']:.1f}, Errors: {summary['errors']}")
                
            except Exception as e:
                print(f"[ERROR] Test failed: {e}")
                import traceback
                traceback.print_exc()
        
        return results
    
    def run_all(self, suite_filter: Optional[str] = None) -> Dict[str, Any]:
        """运行所有测试套件"""
        all_results = []
        suite_results = {}
        
        suites_to_run = [suite_filter] if suite_filter else self.config['TestSuites'].keys()
        
        for suite_name in suites_to_run:
            if suite_name not in self.config['TestSuites']:
                print(f"[WARN] Skipping unknown suite: {suite_name}")
                continue
            
            results = self.run_suite(suite_name)
            all_results.extend(results)
            suite_results[suite_name] = [r.summary() for r in results]
        
        # 生成汇总报告
        report = {
            "metadata": {
                "test_name": self.config['Metadata']['name'],
                "timestamp": datetime.now().isoformat(),
                "config": self.config['TestConfig']
            },
            "summary": self._generate_summary(all_results),
            "suite_results": suite_results,
            "all_results": [r.summary() for r in all_results]
        }
        
        return report
    
    def _generate_summary(self, results: List[BenchmarkResult]) -> Dict[str, Any]:
        """生成测试汇总"""
        if not results:
            return {"error": "No results"}
        
        # 按类别分组
        by_category = {}
        for r in results:
            cat = r.category
            if cat not in by_category:
                by_category[cat] = []
            by_category[cat].append(r)
        
        category_summary = {}
        for cat, cat_results in by_category.items():
            all_latencies = []
            total_errors = 0
            for r in cat_results:
                all_latencies.extend(r.latencies)
                total_errors += r.errors
            
            if all_latencies:
                category_summary[cat] = {
                    "avg_latency_ms": statistics.mean(all_latencies),
                    "median_latency_ms": statistics.median(all_latencies),
                    "min_latency_ms": min(all_latencies),
                    "max_latency_ms": max(all_latencies),
                    "total_commands": len(all_latencies),
                    "total_errors": total_errors,
                    "avg_fps": 1000.0 / statistics.mean(all_latencies)
                }
        
        return {
            "total_tests": len(results),
            "total_commands": sum(len(r.latencies) for r in results),
            "total_errors": sum(r.errors for r in results),
            "category_summary": category_summary
        }


def main():
    parser = argparse.ArgumentParser(description='UnrealCV Extended Benchmark Suite')
    parser.add_argument('--config', default='benchmark_extended.json', 
                        help='Path to benchmark configuration file')
    parser.add_argument('--report', default='benchmark_report.json',
                        help='Path to output report file')
    parser.add_argument('--suite', default=None,
                        help='Run specific test suite only')
    parser.add_argument('--port', type=int, default=9000,
                        help='UnrealCV server port')
    parser.add_argument('--ip', default='127.0.0.1',
                        help='UnrealCV server IP')
    parser.add_argument('--binary', default=None,
                        help='Path to UE binary (auto-launch if provided)')
    parser.add_argument('--map', default='Old_Town',
                        help='Map name to load')
    parser.add_argument('--text', action='store_true',
                        help='Also output text report')
    
    args = parser.parse_args()
    
    # 自动启动 binary
    ue_binary = None
    if args.binary:
        print(f"[INFO] Starting UE binary: {args.binary}")
        ue_binary = RunUnreal(ENV_BIN=args.binary, ENV_MAP=args.map)
        env_ip, env_port = ue_binary.start()
        args.ip = env_ip
        args.port = env_port
    
    try:
        # 运行测试
        benchmark = ExtendedBenchmark(args.config)
        benchmark.connect(args.port, args.ip)
        
        report = benchmark.run_all(args.suite)
        
        # 保存报告
        with open(args.report, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"\n{'='*60}")
        print(f"Benchmark Complete!")
        print(f"Report saved to: {args.report}")
        print(f"{'='*60}")
        
        # 打印汇总
        summary = report['summary']
        print(f"\nTotal Tests: {summary['total_tests']}")
        print(f"Total Commands: {summary['total_commands']}")
        print(f"Total Errors: {summary['total_errors']}")
        
        print("\nCategory Performance:")
        for cat, stats in summary.get('category_summary', {}).items():
            print(f"  {cat:20s}: Avg={stats['avg_latency_ms']:7.2f}ms, "
                  f"FPS={stats['avg_fps']:6.1f}, Commands={stats['total_commands']}")
        
        # 文本报告
        if args.text:
            text_report = args.report.replace('.json', '.txt')
            with open(text_report, 'w') as f:
                f.write(f"UnrealCV Extended Benchmark Report\n")
                f.write(f"Generated: {report['metadata']['timestamp']}\n")
                f.write(f"{'='*60}\n\n")
                
                for suite_name, results in report['suite_results'].items():
                    f.write(f"\n## Suite: {suite_name}\n")
                    for r in results:
                        f.write(f"  {r['cmd']:50s}: {r['avg_latency_ms']:7.2f}ms "
                                f"({r['fps']:6.1f} FPS)\n")
            print(f"Text report saved to: {text_report}")
        
    finally:
        client.disconnect()
        if ue_binary:
            ue_binary.close()


if __name__ == '__main__':
    main()
