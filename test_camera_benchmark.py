#!/usr/bin/env python3
"""
UnrealCV 相机功能综合测试脚本
测试内容：
1. 生成新相机
2. 相机控制移动与状态读取
3. 多模态图像捕获性能测试（FPS）
4. 图像拼接与保存

作者: AI Assistant
日期: 2026-03-05
"""

import sys
import time
import json
import argparse
import numpy as np
from datetime import datetime
from PIL import Image
import io

# 添加 unrealcv 路径
sys.path.insert(0, '/Users/funway/.openclaw/workspace/unrealcv/client/python')

from unrealcv import client
from unrealcv.util import read_png, read_npy


class CameraBenchmark:
    """UnrealCV 相机功能性能测试类"""
    
    def __init__(self, ip='127.0.0.1', port=9000, output_dir='./benchmark_output'):
        self.ip = ip
        self.port = port
        self.output_dir = output_dir
        self.results = {}
        self.saved_images = {}
        
        # 测试配置
        self.test_duration = 5  # 秒
        self.camera_id = 0
        self.new_camera_id = None
        
    def connect(self):
        """连接到 UnrealCV 服务器"""
        print(f"🔗 连接到 UnrealCV 服务器 {self.ip}:{self.port}...")
        client.connect((self.ip, self.port))
        
        # 测试连接
        status = client.request('vget /unrealcv/status')
        print(f"✅ 连接成功! 状态: {status}")
        
        # 获取初始相机数量
        cameras = client.request('vget /cameras').split()
        print(f"📷 当前相机数量: {len(cameras)}")
        
        return True
    
    def disconnect(self):
        """断开连接"""
        client.disconnect()
        print("🔌 已断开连接")
    
    def spawn_new_camera(self):
        """测试1: 生成新相机"""
        print("\n" + "="*60)
        print("📸 测试1: 生成新相机")
        print("="*60)
        
        # 获取当前相机数量
        cameras_before = client.request('vget /cameras').split()
        num_before = len(cameras_before)
        print(f"生成前相机数量: {num_before}")
        
        # 生成新相机
        tic = time.time()
        result = client.request('vset /cameras/spawn')
        spawn_time = (time.time() - tic) * 1000  # ms
        
        # 获取新相机数量
        cameras_after = client.request('vget /cameras').split()
        num_after = len(cameras_after)
        
        self.new_camera_id = num_after - 1
        
        print(f"✅ 新相机生成成功!")
        print(f"   新相机 ID: {self.new_camera_id}")
        print(f"   生成耗时: {spawn_time:.2f} ms")
        print(f"   相机总数: {num_after}")
        
        self.results['spawn_camera'] = {
            'spawn_time_ms': spawn_time,
            'camera_id': self.new_camera_id,
            'total_cameras': num_after
        }
        
        return self.new_camera_id
    
    def test_camera_control(self):
        """测试2: 相机控制移动与状态读取"""
        print("\n" + "="*60)
        print("🎮 测试2: 相机控制移动与状态读取")
        print("="*60)
        
        cam_id = self.camera_id
        
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
        
        # 验证新位置
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
        
        cam_id = self.camera_id
        duration = self.test_duration
        
        # 定义测试的模态
        viewmodes = [
            ('lit', 'png', '彩色图像 (lit)'),
            ('depth', 'npy', '深度图像 (depth)'),
            ('object_mask', 'png', '物体分割 (object_mask)'),
            ('normal', 'png', '法向图像 (normal)'),
            ('optical_flow', 'png', '光流图像 (optical_flow)')
        ]
        
        results = {}
        
        for viewmode, mode, name in viewmodes:
            print(f"\n📷 测试: {name}")
            print(f"   命令: vget /camera/{cam_id}/{viewmode} {mode}")
            print(f"   测试时长: {duration} 秒...", end=' ')
            
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
        
        # 创建网格布局 (2行3列，最后一格放汇总信息)
        cols = 3
        rows = (len(resized_images) + cols - 1) // cols
        
        grid_width = target_size[0] * cols
        grid_height = target_size[1] * rows + 50  # 额外空间给标题
        
        grid = Image.new('RGB', (grid_width, grid_height), (40, 40, 40))
        
        # 粘贴图像
        from PIL import ImageDraw, ImageFont
        draw = ImageDraw.Draw(grid)
        
        try:
            font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 20)
        except:
            font = ImageFont.load_default()
        
        for i, (img, label) in enumerate(zip(resized_images, labels)):
            row = i // cols
            col = i % cols
            x = col * target_size[0]
            y = row * target_size[1] + 40
            
            grid.paste(img, (x, y))
            
            # 添加标签
            label_text = f"{label}"
            bbox = draw.textbbox((0, 0), label_text, font=font)
            text_width = bbox[2] - bbox[0]
            text_x = x + (target_size[0] - text_width) // 2
            draw.text((text_x, y - 30), label_text, fill=(255, 255, 255), font=font)
            
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
            'server': f'{self.ip}:{self.port}',
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
        import os
        os.makedirs(self.output_dir, exist_ok=True)
        
        print("\n🚀 UnrealCV 相机功能综合测试")
        print(f"   时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"   服务器: {self.ip}:{self.port}")
        print(f"   输出目录: {self.output_dir}")
        
        try:
            self.connect()
            self.spawn_new_camera()
            self.test_camera_control()
            self.test_multimodal_capture()
            self.create_image_grid()
            self.generate_report()
            
            print("\n✅ 所有测试完成!")
            return True
            
        except Exception as e:
            print(f"\n❌ 测试失败: {e}")
            import traceback
            traceback.print_exc()
            return False
        finally:
            self.disconnect()


def main():
    parser = argparse.ArgumentParser(description='UnrealCV 相机功能综合测试')
    parser.add_argument('--ip', default='127.0.0.1', help='UnrealCV 服务器IP')
    parser.add_argument('--port', type=int, default=9000, help='UnrealCV 服务器端口')
    parser.add_argument('--output', default='./benchmark_output', help='输出目录')
    parser.add_argument('--duration', type=int, default=5, help='每项测试时长(秒)')
    
    args = parser.parse_args()
    
    benchmark = CameraBenchmark(
        ip=args.ip,
        port=args.port,
        output_dir=args.output
    )
    benchmark.test_duration = args.duration
    
    success = benchmark.run_all_tests()
    
    return 0 if success else 1


if __name__ == '__main__':
    exit(main())
