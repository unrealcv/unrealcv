#!/usr/bin/env python3
"""
Test UnrealCV Performance for test57 (New Build)
"""

import sys
import os
import time
import json
import subprocess
import atexit
from datetime import datetime

# 添加 unrealcv client 路径
sys.path.insert(0, '/Users/funway/.openclaw/workspace/unrealcv/client/python')

import unrealcv
from PIL import Image
import numpy as np


class Test57Benchmark:
    """test57 项目 UnrealCV 性能测试"""
    
    def __init__(self):
        # 优先使用 PackagedDev，其次是 Shipping，最后是 Development
        self.binary_path = self._find_binary()
        self.output_dir = '/Users/funway/.openclaw/workspace/unrealcv/test57_benchmark'
        os.makedirs(self.output_dir, exist_ok=True)
        
        self.resolution = (640, 480)
        self.port = 9000
        self.ip = '127.0.0.1'
        self.test_duration = 5
        
        self.results = {}
        self.saved_images = {}
        self.env_process = None
        self.client = None
    
    def _find_binary(self):
        """查找可用的 test57 binary (优先 PackagedDev)"""
        search_paths = [
            # PackagedDev (推荐)
            '/Volumes/UD210/test57/PackagedDev/Mac/test57.app',
            # Shipping
            '/Volumes/UD210/test57/Binaries/Mac/test57-Mac-Shipping.app',
            # Development
            '/Volumes/UD210/test57/Binaries/Mac/test57.app',
        ]
        
        for path in search_paths:
            if os.path.exists(path):
                print(f"✅ 找到 binary: {path}")
                return path
        
        # 默认返回 PackagedDev 路径
        return '/Volumes/UD210/test57/PackagedDev/Mac/test57.app'
        
    def start_binary(self):
        """启动 test57 binary"""
        # 根据 binary 路径确定配置名称
        config_name = "PackagedDev"
        if "Shipping" in self.binary_path:
            config_name = "Shipping"
        elif "Development" in self.binary_path or "/Binaries/Mac/test57.app" in self.binary_path:
            config_name = "Development"
        
        print(f"🚀 启动 test57 {config_name} 版本...")
        
        # 配置 unrealcv.ini
        self._configure_ini()
        
        # 动态查找可执行文件
        exe_path = self._find_executable()
        
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
    
    def _find_executable(self):
        """查找可执行文件"""
        app_name = os.path.basename(self.binary_path).replace('.app', '')
        
        # 可能的可执行文件名
        possible_names = [
            app_name,  # test57-Mac-Shipping
            'test57',  # test57
        ]
        
        for name in possible_names:
            exe_path = os.path.join(self.binary_path, f'Contents/MacOS/{name}')
            if os.path.exists(exe_path):
                return exe_path
        
        # 默认返回
        return os.path.join(self.binary_path, f'Contents/MacOS/{app_name}')
    
    def _configure_ini(self):
        """配置 unrealcv.ini"""
        # 查找 unrealcv.ini
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
        """断开连接"""
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
        
        self.results['camera_spawn'] = {
            'time_ms': elapsed,
            'before': before,
            'after': after
        }
        
        print(f"   耗时: {elapsed:.2f} ms")
        print(f"   相机数: {before} → {after}")
    
    def test_camera_control(self):
        """测试相机控制"""
        print("\n🎮 测试: 相机控制")
        
        # 读取位置
        tic = time.time()
        loc = self.client.request('vget /camera/0/location')
        get_loc = (time.time() - tic) * 1000
        
        # 设置位置
        loc_vals = [float(x) for x in loc.split()]
        new_loc = [loc_vals[0] + 100, loc_vals[1], loc_vals[2]]
        
        tic = time.time()
        self.client.request(f'vset /camera/0/location {new_loc[0]} {new_loc[1]} {new_loc[2]}')
        set_loc = (time.time() - tic) * 1000
        
        # 批量命令
        tic = time.time()
        cmds = ['vget /camera/0/location', 'vget /camera/0/rotation', 'vget /camera/0/fov']
        self.client.request(cmds)
        batch = (time.time() - tic) * 1000
        
        # 恢复
        self.client.request(f'vset /camera/0/location {loc_vals[0]} {loc_vals[1]} {loc_vals[2]}')
        
        self.results['camera_control'] = {
            'get_location_ms': get_loc,
            'set_location_ms': set_loc,
            'batch_3cmds_ms': batch
        }
        
        print(f"   读取位置: {get_loc:.2f} ms")
        print(f"   设置位置: {set_loc:.2f} ms")
        print(f"   批量命令: {batch:.2f} ms")
    
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
        
        # 保存样本
        if sample:
            path = f'{self.output_dir}/{viewmode}_sample.{fmt}'
            with open(path, 'wb') as f:
                if isinstance(sample, bytes):
                    f.write(sample)
                else:
                    f.write(sample.encode())
            self.saved_images[viewmode] = path
        
        return {'fps': fps, 'count': count, 'duration': elapsed}
    
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
        
        # 创建网格
        n = len(images)
        cols = min(2, n)
        rows = (n + cols - 1) // cols
        
        grid_w = 640 * cols
        grid_h = 480 * rows + 50
        
        grid = Image.new('RGB', (grid_w, grid_h), (30, 30, 30))
        
        from PIL import ImageDraw, ImageFont
        draw = ImageDraw.Draw(grid)
        
        try:
            font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 20)
            title_font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 22)
        except:
            font = ImageFont.load_default()
            title_font = font
        
        # 标题
        title = f"test57 + UnrealCV (PR Merged) - {datetime.now().strftime('%Y-%m-%d %H:%M')}"
        bbox = draw.textbbox((0, 0), title, font=title_font)
        draw.text(((grid_w - (bbox[2]-bbox[0]))//2, 10), title, fill=(255, 255, 255), font=title_font)
        
        # 粘贴图像
        for i, (img, label, fps) in enumerate(zip(images, labels, fps_vals)):
            row = i // cols
            col = i % cols
            x = col * 640
            y = row * 480 + 50
            
            grid.paste(img, (x, y))
            
            text = f"{label} - {fps:.1f} FPS"
            draw.text((x + 10, y + 10), text, fill=(0, 255, 0), font=font)
        
        # 保存
        grid_path = os.path.join(self.output_dir, f'test57_report_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png')
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
            'resolution': self.resolution,
            'results': self.results
        }
        
        # JSON
        json_path = os.path.join(self.output_dir, 'test57_report.json')
        with open(json_path, 'w') as f:
            json.dump(report, f, indent=2, default=str)
        
        # 打印摘要
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
        print("🚀 test57 + UnrealCV (合并 PR 后) 性能测试")
        print("="*60)
        print(f"   Binary: {self.binary_path}")
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
    benchmark = Test57Benchmark()
    grid_path = benchmark.run()
    return 0 if grid_path else 1


if __name__ == '__main__':
    exit(main())
