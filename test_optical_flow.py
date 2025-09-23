#!/usr/bin/env python3
"""
UnrealCV 光流功能测试脚本

此脚本用于验证UnrealCV的光流捕获功能是否正常工作。
测试包括：
1. 连接到UnrealCV服务器
2. 切换到光流视图模式进行可视化测试
3. 通过API获取光流数据
4. 保存光流图像进行分析

使用方法:
1. 启动包含UnrealCV插件的Unreal Engine项目
2. 确保项目中有移动的相机或对象
3. 运行此脚本: python test_optical_flow.py
"""

import cv2
import numpy as np
import time
import os
from pathlib import Path

try:
    import unrealcv
except ImportError:
    print("错误: 请先安装unrealcv库")
    print("安装命令: pip install unrealcv")
    exit(1)

class OpticalFlowTester:
    def __init__(self, host='localhost', port=9000):
        """初始化光流测试器"""
        self.client = unrealcv.Client((host, port))
        self.output_dir = Path("optical_flow_test_output")
        self.output_dir.mkdir(exist_ok=True)

    def connect(self):
        """连接到UnrealCV服务器"""
        print("正在连接到UnrealCV服务器...")
        try:
            self.client.connect()
            if self.client.isconnected():
                print("✅ 成功连接到UnrealCV服务器")
                return True
            else:
                print("❌ 无法连接到UnrealCV服务器")
                return False
        except Exception as e:
            print(f"❌ 连接失败: {e}")
            return False

    def test_optical_flow_viewmode(self):
        """测试光流视图模式"""
        print("\n🔍 测试1: 光流视图模式")
        try:
            # 保存当前视图模式
            current_mode = self.client.request('vget /viewmode')
            print(f"当前视图模式: {current_mode}")

            # 切换到光流模式
            print("切换到光流视图模式...")
            response = self.client.request('vset /viewmode optical_flow')
            print(f"视图模式切换响应: {response}")

            # 等待渲染
            time.sleep(2)

            # 验证模式切换
            new_mode = self.client.request('vget /viewmode')
            print(f"新视图模式: {new_mode}")

            if 'optical_flow' in str(new_mode).lower():
                print("✅ 光流视图模式设置成功")

                # 截取光流视图
                print("截取光流视图图像...")
                screenshot = self.client.request('vget /camera/0/lit')
                if screenshot:
                    screenshot_path = self.output_dir / "optical_flow_viewmode.png"
                    with open(screenshot_path, 'wb') as f:
                        f.write(screenshot)
                    print(f"✅ 光流视图截图已保存: {screenshot_path}")
                else:
                    print("⚠️ 无法获取光流视图截图")

                # 恢复原始视图模式
                self.client.request(f'vset /viewmode {current_mode}')
                return True
            else:
                print(f"❌ 光流视图模式设置失败，当前模式: {new_mode}")
                return False

        except Exception as e:
            print(f"❌ 光流视图模式测试失败: {e}")
            return False

    def test_optical_flow_api(self):
        """测试光流API数据获取"""
        print("\n📊 测试2: 光流API数据获取")
        try:
            # 尝试获取光流数据
            print("请求光流数据...")

            # 由于GetFlow可能还没有直接的API命令，我们先尝试一些可能的命令
            possible_commands = [
                'vget /camera/0/flow',
                'vget /camera/0/optical_flow',
                'vget /camera/0/motion_vector',
                'vget /fusion/flow'
            ]

            flow_data = None
            successful_command = None

            for cmd in possible_commands:
                try:
                    print(f"尝试命令: {cmd}")
                    response = self.client.request(cmd)
                    if response and response != 'error':
                        flow_data = response
                        successful_command = cmd
                        print(f"✅ 命令成功: {cmd}")
                        break
                    else:
                        print(f"⚠️ 命令返回错误或空数据: {cmd}")
                except Exception as e:
                    print(f"⚠️ 命令失败 {cmd}: {e}")

            if flow_data and successful_command:
                print(f"✅ 光流数据获取成功，使用命令: {successful_command}")

                # 保存原始数据
                data_path = self.output_dir / "optical_flow_data.raw"
                if isinstance(flow_data, bytes):
                    with open(data_path, 'wb') as f:
                        f.write(flow_data)
                    print(f"✅ 原始光流数据已保存: {data_path}")

                    # 尝试解析为图像
                    try:
                        self.parse_flow_data(flow_data)
                    except Exception as e:
                        print(f"⚠️ 光流数据解析失败: {e}")
                else:
                    print(f"数据类型: {type(flow_data)}, 长度: {len(str(flow_data))}")

                return True
            else:
                print("❌ 所有光流API命令都失败了")
                print("这可能意味着:")
                print("  1. 光流功能未正确初始化")
                print("  2. API命令名称需要更新")
                print("  3. 需要在C++中添加对应的API处理器")
                return False

        except Exception as e:
            print(f"❌ 光流API测试失败: {e}")
            return False

    def parse_flow_data(self, data):
        """解析光流数据为图像"""
        print("解析光流数据...")

        # 假设数据是PNG格式
        try:
            # 保存为图像文件
            flow_image_path = self.output_dir / "optical_flow_data.png"
            with open(flow_image_path, 'wb') as f:
                f.write(data)

            # 使用OpenCV读取
            flow_img = cv2.imread(str(flow_image_path))
            if flow_img is not None:
                height, width = flow_img.shape[:2]
                print(f"✅ 光流图像解析成功: {width}x{height}")

                # 创建分析图像
                self.analyze_flow_image(flow_img)
                return True
            else:
                print("❌ 无法解析光流图像数据")
                return False

        except Exception as e:
            print(f"❌ 光流数据解析失败: {e}")
            return False

    def analyze_flow_image(self, flow_img):
        """分析光流图像"""
        print("分析光流图像...")

        # 转换为HSV来分析光流
        hsv_img = cv2.cvtColor(flow_img, cv2.COLOR_BGR2HSV)

        # 分析颜色分布
        hue = hsv_img[:, :, 0]
        saturation = hsv_img[:, :, 1]
        value = hsv_img[:, :, 2]

        print(f"色调范围: {hue.min()} - {hue.max()}")
        print(f"饱和度范围: {saturation.min()} - {saturation.max()}")
        print(f"亮度范围: {value.min()} - {value.max()}")

        # 检查是否有运动（非零的饱和度）
        motion_pixels = np.sum(saturation > 10)  # 假设饱和度>10表示有运动
        total_pixels = saturation.size
        motion_percentage = (motion_pixels / total_pixels) * 100

        print(f"运动像素比例: {motion_percentage:.2f}%")

        if motion_percentage > 1:  # 至少1%的像素有运动
            print("✅ 检测到光流运动")
        else:
            print("⚠️ 未检测到明显的光流运动（场景可能是静态的）")

        # 保存分析结果图像
        analysis_path = self.output_dir / "optical_flow_analysis.png"
        cv2.imwrite(str(analysis_path), flow_img)
        print(f"✅ 光流分析图像已保存: {analysis_path}")

    def test_camera_movement(self):
        """测试相机运动时的光流"""
        print("\n🎥 测试3: 相机运动光流")
        try:
            # 获取当前相机位置
            current_pos = self.client.request('vget /camera/0/location')
            print(f"当前相机位置: {current_pos}")

            # 设置光流视图模式
            self.client.request('vset /viewmode optical_flow')
            time.sleep(1)

            # 移动相机并捕获光流
            movements = [
                (100, 0, 0),   # 向前移动
                (-200, 0, 0),  # 向后移动
                (100, 100, 0), # 向右移动
                (0, -100, 0),  # 向左移动
            ]

            for i, (dx, dy, dz) in enumerate(movements):
                print(f"执行运动 {i+1}: dx={dx}, dy={dy}, dz={dz}")

                # 移动相机
                self.client.request(f'vset /camera/0/moveto {dx} {dy} {dz}')
                time.sleep(1)  # 等待运动完成

                # 捕获光流
                screenshot = self.client.request('vget /camera/0/lit')
                if screenshot:
                    screenshot_path = self.output_dir / f"flow_movement_{i+1}.png"
                    with open(screenshot_path, 'wb') as f:
                        f.write(screenshot)
                    print(f"✅ 运动 {i+1} 光流图像已保存: {screenshot_path}")

                time.sleep(0.5)

            # 恢复相机位置
            self.client.request(f'vset /camera/0/location {current_pos}')
            self.client.request('vset /viewmode lit')

            return True

        except Exception as e:
            print(f"❌ 相机运动测试失败: {e}")
            return False

    def run_all_tests(self):
        """运行所有测试"""
        print("🚀 开始UnrealCV光流功能测试")
        print("=" * 50)

        if not self.connect():
            return False

        results = []

        # 测试1: 视图模式
        results.append(("光流视图模式", self.test_optical_flow_viewmode()))

        # 测试2: API数据获取
        results.append(("光流API数据", self.test_optical_flow_api()))

        # 测试3: 相机运动
        results.append(("相机运动光流", self.test_camera_movement()))

        # 输出总结
        print("\n" + "=" * 50)
        print("🏁 测试总结:")
        print("=" * 50)

        passed = 0
        total = len(results)

        for test_name, result in results:
            status = "✅ 通过" if result else "❌ 失败"
            print(f"{test_name}: {status}")
            if result:
                passed += 1

        print(f"\n总体结果: {passed}/{total} 测试通过")

        if passed == total:
            print("🎉 所有测试通过！UnrealCV光流功能正常工作")
        elif passed > 0:
            print("⚠️ 部分测试通过，光流功能可能需要进一步调试")
        else:
            print("❌ 所有测试失败，光流功能需要修复")

        print(f"\n测试输出文件保存在: {self.output_dir.absolute()}")

        return passed == total

if __name__ == "__main__":
    # 运行测试
    tester = OpticalFlowTester()
    success = tester.run_all_tests()

    print("\n📋 下一步建议:")
    if success:
        print("1. 检查保存的光流图像，验证视觉效果")
        print("2. 在你的应用中集成光流功能")
        print("3. 根据需要调整光流材质参数")
    else:
        print("1. 检查UnrealCV服务器是否正在运行")
        print("2. 确认OpticalFlowMaterial.uasset在正确路径")
        print("3. 检查Unreal Engine控制台是否有错误信息")
        print("4. 验证光流API命令是否已在C++中实现")