#!/usr/bin/env python3
"""
Test UnrealCV Performance for test57 Shipping Version
"""

import sys
import os
import time
import json
import subprocess
import atexit
from datetime import datetime

sys.path.insert(0, '/Users/funway/.openclaw/workspace/unrealcv/client/python')

import unrealcv
from PIL import Image
import numpy as np


class Test57ShippingBenchmark:
    """test57 Shipping 版本性能测试"""
    
    def __init__(self):
        # Shipping binary 路径
        self.binary_path = '/Volumes/UD210/test57/Binaries/Mac/test57-Mac-Shipping.app'
        self.output_dir = '/Users/funway/.openclaw/workspace/unrealcv/test57_benchmark_shipping'
        os.makedirs(self.output_dir, exist_ok=True)
        
        self.resolution = (640, 480)
        self.port = 9000
        self.ip = '127.0.0.1'
        self.test_duration = 5
        
        self.results = {}
        self.saved_images = {}
        self.env_process = None
        self.client = None
        
    def start_binary(self):
        """启动 Shipping binary"""
        print("🚀 启动 test57 Shipping 版本...")
        
        self._configure_ini()
        
        exe_path = os.path.join(self.binary_path, 'Contents/MacOS/test57-Mac-Shipping')
        
        self.env_process = subprocess.Popen(
            [exe_path],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        
        atexit.register(self.stop_binary)
        
        print(f"   PID: {self.env_process.pid}")
        print("⏳ 等待启动 (10秒)...")
        time.sleep(10)
        
        print("✅ Binary 启动完成")
        return True
    
    def _configure_ini(self):
        """配置 unrealcv.ini"""
        import glob
        ini_pattern = os.path.join(self.binary_path, 'Contents/UE*/test57/Binaries/Mac/unrealcv.ini')
        ini_files = glob.glob(ini_pattern)
        
        if ini_files:
            ini_file = ini_files[0]
            print(f"   配置: {ini_file}")
            
            with open(ini_file, 'r') as f:
                lines = f.read().split('\n')
            
            with open(ini_file, 'w') as f:
                for i, line in enumerate(lines):
                    if line.startswith('Port='):
                        lines[i] = f'Port={self.port}'
                    elif line.startswith('Width='):
                        lines[i] = f'Width={self.resolution[0]}'
                    elif line.startswith('Height='):
                        lines[i] = f'Height={self.resolution[1]}'
                f.write('\n'.join(lines))
    
    def stop_binary(self):
        """停止 binary"""
        if self.env_process:
            print("\n🛑 停止 test57...")
            self.env_process.terminate()
            self.env_process.wait()
            print("✅ Binary 已停止")
            self.env_process = None
    
    def connect(self):
        """连接到 UnrealCV"""
        print(f"\n🔗 连接到 UnrealCV {self.ip}:{self.port}...")
        
        max_retries = 15
        for i in range(max_retries):
            try:
                self.client = unrealcv.Client((self.ip, self.port))
                if self.client.connect():
                    status = self.client.request('vget /unrealcv/status')
                    print(f"✅ 连接成功!")
                    print(f"   状态: {status}")
                    return True
            except Exception as e:
                print(f"   重试 {i+1}/{max_retries}...")
                time.sleep(1)
        
        raise Exception("无法连接")
    
    def disconnect(self):
        if self.client:
            self.client.disconnect()
            self.client = None
            print("🔌 已断开连接")
    
    def test_camera_spawn(self):
        """测试生成相机"""
        print("\n📸 测试: 生成新相机")
        
        before = len(self.client.request('vget /cameras').split())
        
        tic = time.time()
        self.client.request('vset /cameras/spawn')
        elapsed = (time.time() - tic) * 1000
        
        after = len(self.client.request('vget /cameras').split())
        
        self.results['camera_spawn'] = {'time_ms': elapsed, 'before': before, 'after': after}
        print(f"   耗时: {elapsed:.2f} ms, 相机数: {before} → {after}")
    
    def test_camera_control(self):
        """测试相机控制"""
        print("\n🎮 测试: 相机控制")
        
        tic = time.time()
        loc = self.client.request('vget /camera/0/location')
        get_loc = (time.time() - tic) * 1000
        
        loc_vals = [float(x) for x in loc.split()]
        new_loc = [loc_vals[0] + 100, loc_vals[1], loc_vals[2]]
        
        tic = time.time()
        self.client.request(f'vset /camera/0/location {new_loc[0]} {new_loc[1]} {new_loc[2]}')
        set_loc = (time.time() - tic) * 1000
        
        tic = time.time()
        cmds = ['vget /camera/0/location', 'vget /camera/0/rotation', 'vget /camera/0/fov']
        self.client.request(cmds)
        batch = (time.time() - tic) * 1000
        
        self.client.request(f'vset /camera/0/location {loc_vals[0]} {loc_vals[1]} {loc_vals[2]}')
        
        self.results['camera_control'] = {
            'get_location_ms': get_loc,
            'set_location_ms': set_loc,
            'batch_3cmds_ms': batch
        }
        print(f"   读取: {get_loc:.2f} ms, 设置: {set_loc:.2f} ms, 批量: {batch:.2f} ms")
    
    def benchmark_viewmode(self, viewmode, fmt='png'):
        """测试视图模式 FPS"""
        cmd = f'vget /camera/0/{viewmode} {fmt}'
        count = 0
        sample = None
        
        tic = time.time()
        while time.time() - tic < self.test_duration:
            res = self.client.request(cmd)
            if sample is None:
                sample = res
            count += 1
        
        elapsed = time.time() - tic
        fps = count / elapsed
        
        if sample:
            path = f'{self.output_dir}/{viewmode}_sample.{fmt}'
            with open(path, 'wb') as f:
                f.write(sample if isinstance(sample, bytes) else sample.encode())
            self.saved_images[viewmode] = path
        
        return {'fps': fps, 'count': count}
    
    def test_multimodal(self):
        """测试多模态捕获"""
        print("\n🖼️  测试: 多模态图像捕获 (5秒)")
        
        viewmodes = [
            ('lit', 'png', '彩色'),
            ('depth', 'npy', '深度'),
            ('object_mask', 'png', '分割'),
            ('normal', 'png', '法向'),
        ]
        
        results = {}
        for vm, fmt, name in viewmodes:
            print(f"   测试 {name}...", end=' ', flush=True)
            try:
                result = self.benchmark_viewmode(vm, fmt)
                results[vm] = result
                print(f"✅ {result['fps']:.2f} FPS")
            except Exception as e:
                print(f"❌ {e}")
                results[vm] = {'error': str(e), 'fps': 0}
        
        self.results['multimodal'] = results
    
    def create_grid(self):
        """创建图像拼接"""
        print("\n🎨 生成可视化报告...")
        
        images = []
        labels = []
        fps_vals = []
        order = ['lit', 'depth', 'object_mask', 'normal']
        
        for vm in order:
            if vm not in self.results.get('multimodal', {}):
                continue
            path = self.saved_images.get(vm)
            if not path:
                continue
            try:
                if path.endswith('.npy'):
                    depth = np.load(path)
                    depth_norm = ((depth - depth.min()) / (depth.max() - depth.min() + 1e-8) * 255).astype(np.uint8)
                    img = Image.fromarray(np.stack([depth_norm]*3, axis=-1))
                else:
                    img = Image.open(path)
                    if img.mode == 'RGBA':
                        img = img.convert('RGB')
                
                img = img.resize((640, 480), Image.Resampling.LANCZOS)
                images.append(img)
                labels.append(vm)
                fps_vals.append(self.results['multimodal'][vm]['fps'])
            except Exception as e:
                print(f"   ⚠️  {vm}: {e}")
        
        if len(images) < 2:
            return None
        
        n = len(images)
        cols = min(2, n)
        rows = (n + cols - 1) // cols
        grid_w, grid_h = 640 * cols, 480 * rows + 50
        
        grid = Image.new('RGB', (grid_w, grid_h), (30, 30, 30))
        from PIL import ImageDraw, ImageFont
        draw = ImageDraw.Draw(grid)
        
        try:
            font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 20)
            title_font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 22)
        except:
            font = title_font = ImageFont.load_default()
        
        title = f"test57 Shipping + UnrealCV - {datetime.now().strftime('%Y-%m-%d %H:%M')}"
        bbox = draw.textbbox((0, 0), title, font=title_font)
        draw.text(((grid_w - (bbox[2]-bbox[0]))//2, 10), title, fill=(255, 255, 255), font=title_font)
        
        for i, (img, label, fps) in enumerate(zip(images, labels, fps_vals)):
            row, col = i // cols, i % cols
            x, y = col * 640, row * 480 + 50
            grid.paste(img, (x, y))
            text = f"{label} - {fps:.1f} FPS"
            draw.text((x + 10, y + 10), text, fill=(0, 255, 0), font=font)
        
        grid_path = os.path.join(self.output_dir, f'test57_shipping_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png')
        grid.save(grid_path, quality=95)
        print(f"✅ 报告保存: {grid_path}")
        self.saved_images['grid'] = grid_path
        return grid_path
    
    def generate_report(self):
        """生成报告"""
        print("\n📊 性能报告")
        print("="*60)
        
        report = {
            'timestamp': datetime.now().isoformat(),
            'binary': self.binary_path,
            'config': 'Shipping',
            'resolution': self.resolution,
            'results': self.results
        }
        
        json_path = os.path.join(self.output_dir, 'test57_shipping_report.json')
        with open(json_path, 'w') as f:
            json.dump(report, f, indent=2, default=str)
        
        print(f"\n📸 相机生成:")
        if 'camera_spawn' in self.results:
            r = self.results['camera_spawn']
            print(f"   耗时: {r['time_ms']:.2f} ms")
        
        print(f"\n🎮 相机控制:")
        if 'camera_control' in self.results:
            r = self.results['camera_control']
            print(f"   读取位置: {r['get_location_ms']:.2f} ms")
            print(f"   设置位置: {r['set_location_ms']:.2f} ms")
            print(f"   批量命令: {r['batch_3cmds_ms']:.2f} ms")
        
        print(f"\n🖼️  多模态捕获 FPS:")
        if 'multimodal' in self.results:
            for vm, data in self.results['multimodal'].items():
                if 'fps' in data:
                    print(f"   {vm:20s}: {data['fps']:6.2f} FPS")
        
        print(f"\n📄 完整报告: {json_path}")
        return json_path
    
    def run(self):
        """运行所有测试"""
        print("\n" + "="*60)
        print("🚀 test57 Shipping + UnrealCV 性能测试")
        print("="*60)
        print(f"   Binary: {self.binary_path}")
        print(f"   配置: Shipping")
        print(f"   时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        
        try:
            self.start_binary()
            self.connect()
            
            self.test_camera_spawn()
            self.test_camera_control()
            self.test_multimodal()
            
            grid_path = self.create_grid()
            report_path = self.generate_report()
            
            print("\n✅ 测试完成!")
            return grid_path
            
        except Exception as e:
            print(f"\n❌ 测试失败: {e}")
            import traceback
            traceback.print_exc()
            return None
        finally:
            self.disconnect()
            self.stop_binary()


def main():
    benchmark = Test57ShippingBenchmark()
    grid_path = benchmark.run()
    return 0 if grid_path else 1


if __name__ == '__main__':
    exit(main())
