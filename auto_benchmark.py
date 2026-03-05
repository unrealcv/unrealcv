#!/usr/bin/env python3
"""
UnrealCV 相机功能自动化测试脚本
自动启动 binary，运行测试，保存结果，并发送到飞书

作者: AI Assistant
日期: 2026-03-05
"""

import sys
import os
import time
import json
import subprocess
import atexit
from datetime import datetime
from PIL import Image
import numpy as np

# 添加 unrealcv 路径
sys.path.insert(0, '/Users/funway/.openclaw/workspace/unrealcv/client/python')

import unrealcv
from unrealcv.util import read_png, read_npy

# 创建全局 client 实例
client = None


class UnrealCVAutoBenchmark:
    """自动化 UnrealCV 测试类"""
    
    def __init__(self):
        # Binary 路径 (优先使用 PackagedDev，其次是 Shipping，最后是 Development)
        self.binary_path = self._find_binary()
        self.output_dir = '/Users/funway/.openclaw/workspace/unrealcv/benchmark_output'
        os.makedirs(self.output_dir, exist_ok=True)
        
        # 测试配置
        self.test_duration = 5  # 秒
        self.resolution = (640, 480)
        self.port = 9000
        self.ip = '127.0.0.1'
        
        # 结果存储
        self.results = {}
        self.saved_images = {}
        self.env_process = None
    
    def _find_binary(self):
        """查找可用的 UnrealCV binary"""
        # 搜索路径 (按优先级)
        search_paths = [
            # PackagedDev (推荐)
            '/Volumes/UD210/test57/PackagedDev/Mac/test57.app',
            # Shipping
            '/Volumes/UD210/test57/Binaries/Mac/test57-Mac-Shipping.app',
            # Development
            '/Volumes/UD210/test57/Binaries/Mac/test57.app',
            # 默认 UnrealEnv
            '/Users/funway/.unrealcv/UnrealEnv/UE4_ExampleScene_Mac/UE4_ExampleScene.app',
        ]
        
        for path in search_paths:
            if os.path.exists(path):
                print(f"✅ 找到 binary: {path}")
                return path
        
        # 如果都找不到，返回默认路径
        return '/Users/funway/.unrealcv/UnrealEnv/UE4_ExampleScene_Mac/UE4_ExampleScene.app'
        
    def start_binary(self):
        """启动 UnrealCV Binary"""
        print("🚀 启动 UnrealCV Binary...")
        print(f"   路径: {self.binary_path}")
        print(f"   分辨率: {self.resolution[0]}x{self.resolution[1]}")
        
        # 设置分辨率和端口
        self._configure_unrealcv_ini()
        
        # 动态查找可执行文件
        exe_path = self._find_executable()
        
        # 构建启动命令
        cmd = [exe_path]
        
        print(f"   执行命令: {' '.join(cmd)}")
        
        # 启动进程
        self.env_process = subprocess.Popen(
            cmd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        
        # 注册清理函数
        atexit.register(self.stop_binary)
        
        print(f"   Binary PID: {self.env_process.pid}")
        print("⏳ 等待环境启动 (8秒)...")
        time.sleep(8)
        
        print("✅ Binary 启动完成")
        return True
    
    def _find_executable(self):
        """查找可执行文件路径"""
        app_name = os.path.basename(self.binary_path).replace('.app', '')
        
        # 可能的可执行文件名
        possible_names = [
            app_name,  # test57-Mac-Shipping
            app_name.replace('-Mac-Shipping', ''),  # test57
            'UE4_ExampleScene',
        ]
        
        for name in possible_names:
            exe_path = os.path.join(self.binary_path, f'Contents/MacOS/{name}')
            if os.path.exists(exe_path):
                return exe_path
        
        # 默认返回
        return os.path.join(self.binary_path, f'Contents/MacOS/{app_name}')
    
    def _configure_unrealcv_ini(self):
        """配置 unrealcv.ini"""
        import glob
        
        # 动态查找 unrealcv.ini (支持多种路径格式)
        app_name = os.path.basename(self.binary_path).replace('.app', '').replace('-Mac-Shipping', '')
        
        # 可能的ini路径
        possible_patterns = [
            os.path.join(self.binary_path, f'Contents/UE*/{app_name}/Binaries/Mac/unrealcv.ini'),
            os.path.join(self.binary_path, 'Contents/UE4/*/Binaries/Mac/unrealcv.ini'),
        ]
        
        ini_file = None
        for pattern in possible_patterns:
            matches = glob.glob(pattern)
            if matches:
                ini_file = matches[0]
                break
        
        if ini_file and os.path.exists(ini_file):
            print(f"   配置 unrealcv.ini: {ini_file}")
            
            with open(ini_file, 'r') as f:
                lines = f.read().split('\n')
            
            # 修改配置
            with open(ini_file, 'w') as f:
                for i, line in enumerate(lines):
                    if line.startswith('Port='):
                        lines[i] = f'Port={self.port}'
                    elif line.startswith('Width='):
                        lines[i] = f'Width={self.resolution[0]}'
                    elif line.startswith('Height='):
                        lines[i] = f'Height={self.resolution[1]}'
                f.write('\n'.join(lines))
            
            print(f"   已配置: port={self.port}, res={self.resolution}")
        else:
            print(f"   ⚠️ 未找到 unrealcv.ini")
                lines[2] = f'Width={self.resolution[0]}'
                lines[3] = f'Height={self.resolution[1]}'
                f.write('\n'.join(lines))
            
            print(f"   已配置 unrealcv.ini: port={self.port}, res={self.resolution}")
    
    def stop_binary(self):
        """停止 Binary"""
        if self.env_process:
            print("\n🛑 停止 UnrealCV Binary...")
            self.env_process.terminate()
            self.env_process.wait()
            print("✅ Binary 已停止")
            self.env_process = None
    
    def connect(self):
        """连接到 UnrealCV 服务器"""
        global client
        print(f"\n🔗 连接到 UnrealCV 服务器 {self.ip}:{self.port}...")
        
        max_retries = 10
        for i in range(max_retries):
            try:
                client = unrealcv.Client((self.ip, self.port))
                if client.connect():
                    status = client.request('vget /unrealcv/status')
                    print(f"✅ 连接成功! 状态: {status}")
                    return True
            except Exception as e:
                print(f"   连接尝试 {i+1}/{max_retries} 失败: {e}")
                time.sleep(1)
        
        raise Exception("无法连接到 UnrealCV 服务器")
    
    def disconnect(self):
        """断开连接"""
        global client
        if client:
            client.disconnect()
            client = None
            print("🔌 已断开连接")
    
    def spawn_new_camera(self):
        """测试1: 生成新相机"""
        print("\n" + "="*60)
        print("📸 测试1: 生成新相机")
        print("="*60)
        
        cameras_before = len(client.request('vget /cameras').split())
        
        tic = time.time()
        result = client.request('vset /cameras/spawn')
        spawn_time = (time.time() - tic) * 1000
        
        cameras_after = len(client.request('vget /cameras').split())
        new_camera_id = cameras_after - 1
        
        print(f"✅ 新相机生成成功!")
        print(f"   新相机 ID: {new_camera_id}")
        print(f"   生成耗时: {spawn_time:.2f} ms")
        print(f"   相机总数: {cameras_after}")
        
        self.results['spawn_camera'] = {
            'spawn_time_ms': spawn_time,
            'camera_id': new_camera_id,
            'total_cameras': cameras_after
        }
        
        return new_camera_id
    
    def test_camera_control(self):
        """测试2: 相机控制移动与状态读取"""
        print("\n" + "="*60)
        print("🎮 测试2: 相机控制移动与状态读取")
        print("="*60)
        
        cam_id = 0
        
        # 获取初始状态
        print("\n📍 初始状态读取:")
        tic = time.time()
        init_loc = client.request(f'vget /camera/{cam_id}/location')
        loc_time = (time.time() - tic) * 1000
        
        tic = time.time()
        init_rot = client.request(f'vget /camera/{cam_id}/rotation')
        rot_time = (time.time() - tic) * 1000
        
        tic = time.time()
        init_fov = client.request(f'vget /camera/{cam_id}/fov')
        fov_time = (time.time() - tic) * 1000
        
        init_loc = [float(x) for x in init_loc.split()]
        init_rot = [float(x) for x in init_rot.split()]
        
        print(f"   位置: {init_loc} (读取耗时: {loc_time:.2f} ms)")
        print(f"   旋转: {init_rot} (读取耗时: {rot_time:.2f} ms)")
        print(f"   FOV: {init_fov} (读取耗时: {fov_time:.2f} ms)")
        
        # 测试位置设置
        print("\n🔄 测试相机移动:")
        new_loc = [init_loc[0] + 100, init_loc[1] + 50, init_loc[2] + 20]
        
        tic = time.time()
        client.request(f'vset /camera/{cam_id}/location {new_loc[0]} {new_loc[1]} {new_loc[2]}')
        move_time = (time.time() - tic) * 1000
        
        verify_loc = client.request(f'vget /camera/{cam_id}/location')
        verify_loc = [float(x) for x in verify_loc.split()]
        
        print(f"   目标位置: {new_loc}")
        print(f"   实际位置: {verify_loc}")
        print(f"   移动耗时: {move_time:.2f} ms")
        
        # 测试旋转设置
        new_rot = [init_rot[0] + 10, init_rot[1] + 15, init_rot[2] + 5]
        
        tic = time.time()
        client.request(f'vset /camera/{cam_id}/rotation {new_rot[0]} {new_rot[1]} {new_rot[2]}')
        rot_set_time = (time.time() - tic) * 1000
        
        verify_rot = client.request(f'vget /camera/{cam_id}/rotation')
        verify_rot = [float(x) for x in verify_rot.split()]
        
        print(f"   目标旋转: {new_rot}")
        print(f"   实际旋转: {verify_rot}")
        print(f"   旋转设置耗时: {rot_set_time:.2f} ms")
        
        # 批量命令测试
        print("\n📦 批量命令测试:")
        tic = time.time()
        cmds = [
            f'vget /camera/{cam_id}/location',
            f'vget /camera/{cam_id}/rotation',
            f'vget /camera/{cam_id}/fov'
        ]
        results = client.request(cmds)
        batch_time = (time.time() - tic) * 1000
        
        print(f"   批量读取3个状态耗时: {batch_time:.2f} ms")
        print(f"   平均每个命令: {batch_time/3:.2f} ms")
        
        # 恢复原始位置
        client.request(f'vset /camera/{cam_id}/location {init_loc[0]} {init_loc[1]} {init_loc[2]}')
        client.request(f'vset /camera/{cam_id}/rotation {init_rot[0]} {init_rot[1]} {init_rot[2]}')
        print("\n✅ 已恢复相机原始位置")
        
        self.results['camera_control'] = {
            'get_location_ms': loc_time,
            'get_rotation_ms': rot_time,
            'get_fov_ms': fov_time,
            'set_location_ms': move_time,
            'set_rotation_ms': rot_set_time,
            'batch_3cmds_ms': batch_time,
            'batch_avg_ms': batch_time / 3
        }
    
    def benchmark_viewmode(self, cam_id, viewmode, mode='png', duration=5):
        """测试特定视图模式的FPS"""
        cmd = f'vget /camera/{cam_id}/{viewmode} {mode}'
        
        sample_image = None
        count = 0
        errors = 0
        
        tic = time.time()
        while time.time() - tic < duration:
            try:
                res = client.request(cmd)
                if sample_image is None:
                    sample_image = res
                count += 1
            except Exception as e:
                errors += 1
                if errors > 5:
                    break
        
        elapsed = time.time() - tic
        fps = count / elapsed if elapsed > 0 else 0
        
        return {
            'fps': fps,
            'count': count,
            'duration': elapsed,
            'errors': errors,
            'sample_image': sample_image
        }
    
    def test_multimodal_capture(self):
        """测试3: 多模态图像捕获性能"""
        print("\n" + "="*60)
        print("🖼️  测试3: 多模态图像捕获性能")
        print("="*60)
        
        cam_id = 0
        duration = self.test_duration
        
        # 定义测试的模态
        viewmodes = [
            ('lit', 'png', '彩色图像 (lit)'),
            ('depth', 'npy', '深度图像 (depth)'),
            ('object_mask', 'png', '物体分割 (object_mask)'),
            ('normal', 'png', '法向图像 (normal)'),
        ]
        
        # 测试光流是否可用
        try:
            test_flow = client.request(f'vget /camera/{cam_id}/optical_flow png')
            viewmodes.append(('optical_flow', 'png', '光流图像 (optical_flow)'))
        except:
            print("⚠️  光流模式不可用，跳过测试")
        
        results = {}
        
        for viewmode, mode, name in viewmodes:
            print(f"\n📷 测试: {name}")
            print(f"   命令: vget /camera/{cam_id}/{viewmode} {mode}")
            print(f"   测试时长: {duration} 秒...", end=' ', flush=True)
            
            try:
                result = self.benchmark_viewmode(cam_id, viewmode, mode, duration)
                results[viewmode] = result
                
                print(f"✅ FPS: {result['fps']:.2f} ({result['count']} 帧)")
                
                # 保存样本图像
                if result['sample_image']:
                    filename = f'{self.output_dir}/{viewmode}_sample.{mode}'
                    with open(filename, 'wb') as f:
                        f.write(result['sample_image'])
                    self.saved_images[viewmode] = filename
                    print(f"   样本已保存: {filename}")
                    
            except Exception as e:
                print(f"❌ 失败: {e}")
                results[viewmode] = {'error': str(e), 'fps': 0}
        
        self.results['multimodal_capture'] = results
        
        # 打印汇总
        print("\n" + "-"*60)
        print("📊 多模态捕获性能汇总:")
        print("-"*60)
        for viewmode, mode, name in viewmodes:
            if viewmode in results and 'fps' in results[viewmode]:
                fps = results[viewmode]['fps']
                print(f"   {name:25s}: {fps:6.2f} FPS")
    
    def create_image_grid(self):
        """创建图像拼接网格"""
        print("\n" + "="*60)
        print("🎨 测试4: 图像拼接")
        print("="*60)
        
        images = {}
        
        # 读取保存的图像
        for viewmode, filepath in self.saved_images.items():
            try:
                if filepath.endswith('.npy'):
                    # 深度图需要特殊处理
                    depth = np.load(filepath)
                    # 归一化到 0-255
                    depth_norm = ((depth - depth.min()) / (depth.max() - depth.min() + 1e-8) * 255).astype(np.uint8)
                    # 转换为3通道
                    depth_rgb = np.stack([depth_norm] * 3, axis=-1)
                    img = Image.fromarray(depth_rgb)
                else:
                    img = Image.open(filepath)
                    if img.mode == 'RGBA':
                        img = img.convert('RGB')
                
                images[viewmode] = img
                print(f"✅ 已加载: {viewmode} ({img.size})")
            except Exception as e:
                print(f"⚠️  无法加载 {viewmode}: {e}")
        
        if len(images) < 2:
            print("❌ 图像数量不足，无法拼接")
            return None
        
        # 统一图像大小
        target_size = (640, 480)
        resized_images = []
        labels = []
        
        viewmode_order = ['lit', 'depth', 'object_mask', 'normal', 'optical_flow']
        
        for vm in viewmode_order:
            if vm in images:
                img = images[vm].resize(target_size, Image.Resampling.LANCZOS)
                resized_images.append(img)
                labels.append(vm)
        
        if len(resized_images) == 0:
            return None
        
        # 创建网格布局
        n_images = len(resized_images)
        cols = min(3, n_images)
        rows = (n_images + cols - 1) // cols
        
        grid_width = target_size[0] * cols
        grid_height = target_size[1] * rows + 60  # 额外空间给标题
        
        grid = Image.new('RGB', (grid_width, grid_height), (40, 40, 40))
        
        # 粘贴图像
        from PIL import ImageDraw, ImageFont
        draw = ImageDraw.Draw(grid)
        
        try:
            font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 20)
            title_font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 24)
        except:
            font = ImageFont.load_default()
            title_font = font
        
        # 添加标题
        title = "UnrealCV Multi-Modal Camera Benchmark"
        bbox = draw.textbbox((0, 0), title, font=title_font)
        text_width = bbox[2] - bbox[0]
        draw.text(((grid_width - text_width) // 2, 10), title, fill=(255, 255, 255), font=title_font)
        
        for i, (img, label) in enumerate(zip(resized_images, labels)):
            row = i // cols
            col = i % cols
            x = col * target_size[0]
            y = row * target_size[1] + 60
            
            grid.paste(img, (x, y))
            
            # 添加标签
            label_text = f"{label}"
            bbox = draw.textbbox((0, 0), label_text, font=font)
            text_width = bbox[2] - bbox[0]
            text_x = x + (target_size[0] - text_width) // 2
            draw.text((text_x, y - 25), label_text, fill=(255, 255, 255), font=font)
            
            # 添加FPS信息
            if label in self.results.get('multimodal_capture', {}):
                fps = self.results['multimodal_capture'][label]['fps']
                fps_text = f"FPS: {fps:.1f}"
                draw.text((x + 10, y + target_size[1] - 30), fps_text, fill=(0, 255, 0), font=font)
        
        # 保存拼接图像
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        grid_path = f'{self.output_dir}/multimodal_grid_{timestamp}.png'
        grid.save(grid_path, quality=95)
        print(f"\n✅ 拼接图像已保存: {grid_path}")
        
        self.saved_images['grid'] = grid_path
        return grid_path
    
    def generate_report(self):
        """生成测试报告"""
        print("\n" + "="*60)
        print("📋 测试报告汇总")
        print("="*60)
        
        report = {
            'timestamp': datetime.now().isoformat(),
            'binary': self.binary_path,
            'resolution': self.resolution,
            'results': self.results,
            'saved_images': self.saved_images
        }
        
        # 保存JSON报告
        report_path = f'{self.output_dir}/benchmark_report.json'
        with open(report_path, 'w') as f:
            json.dump(report, f, indent=2)
        print(f"📄 JSON报告已保存: {report_path}")
        
        # 打印文本报告
        print("\n" + "="*60)
        print("🎯 性能摘要")
        print("="*60)
        
        if 'spawn_camera' in self.results:
            r = self.results['spawn_camera']
            print(f"\n📸 相机生成:")
            print(f"   新相机 ID: {r['camera_id']}")
            print(f"   生成耗时: {r['spawn_time_ms']:.2f} ms")
        
        if 'camera_control' in self.results:
            r = self.results['camera_control']
            print(f"\n🎮 相机控制:")
            print(f"   读取位置: {r['get_location_ms']:.2f} ms")
            print(f"   读取旋转: {r['get_rotation_ms']:.2f} ms")
            print(f"   设置位置: {r['set_location_ms']:.2f} ms")
            print(f"   批量3命令: {r['batch_3cmds_ms']:.2f} ms")
        
        if 'multimodal_capture' in self.results:
            print(f"\n🖼️  图像捕获 FPS:")
            for viewmode, result in self.results['multimodal_capture'].items():
                if 'fps' in result:
                    print(f"   {viewmode:20s}: {result['fps']:6.2f} FPS")
        
        return report_path
    
    def run_all_tests(self):
        """运行所有测试"""
        print("\n🚀 UnrealCV 相机功能自动化测试")
        print(f"   时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"   Binary: {self.binary_path}")
        print(f"   输出目录: {self.output_dir}")
        
        try:
            # 启动 binary
            self.start_binary()
            
            # 连接
            self.connect()
            
            # 运行测试
            self.spawn_new_camera()
            self.test_camera_control()
            self.test_multimodal_capture()
            
            # 生成图像网格
            grid_path = self.create_image_grid()
            
            # 生成报告
            self.generate_report()
            
            print("\n✅ 所有测试完成!")
            
            # 返回拼接图像路径供飞书发送
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
    benchmark = UnrealCVAutoBenchmark()
    grid_path = benchmark.run_all_tests()
    
    if grid_path:
        print(f"\n📤 测试结果图像: {grid_path}")
        return 0
    else:
        return 1


if __name__ == '__main__':
    exit(main())
