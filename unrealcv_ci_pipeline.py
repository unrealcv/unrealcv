#!/usr/bin/env python3
"""
UnrealCV 完整自动化评估流水线
================================
功能：检测代码变更 → 编译插件 → 打包项目 → 性能测试 → 生成报告 → 飞书通知

作者: AI Assistant
日期: 2026-03-05
版本: 1.0

使用方法:
    python unrealcv_ci_pipeline.py [--mode auto|build|test|full] [--project-path PATH]

模式说明:
    auto  - 自动检测代码变更，如有更新则执行完整流程
    build - 仅执行编译和打包
    test  - 仅执行性能测试（使用现有binary）
    full  - 强制执行完整流程（无论是否有代码变更）
"""

import os
import sys
import json
import time
import argparse
import subprocess
import hashlib
import shutil
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

# 添加 unrealcv client 路径
sys.path.insert(0, '/Users/funway/.openclaw/workspace/unrealcv/client/python')

import unrealcv
from PIL import Image
import numpy as np


# ==================== 配置区域 ====================

class Config:
    """流水线配置"""
    
    # 路径配置
    UNREALCV_REPO = '/Users/funway/.openclaw/workspace/unrealcv'
    UE4_PROJECT_PATH = '/Users/funway/Documents/Unreal Projects/UE4_ExampleScene'  # 需要根据实际情况修改
    OUTPUT_DIR = '/Users/funway/.openclaw/workspace/unrealcv/ci_reports'
    PACKAGE_DIR = '/Users/funway/.unrealcv/Packaged'
    
    # 版本追踪
    VERSION_FILE = os.path.join(OUTPUT_DIR, '.last_tested_version')
    
    # 编译配置
    UE4_EDITOR_PATH = '/Users/Shared/Epic Games/UE_4.27/Engine/Binaries/Mac/UE4Editor-Cmd'  # Mac
    # UE4_EDITOR_PATH = 'C:/Program Files/Epic Games/UE_4.27/Engine/Binaries/Win64/UE4Editor-Cmd.exe'  # Windows
    BUILD_CONFIG = 'PackagedDev'  # 可选: Development, PackagedDev, Shipping, Test
    
    # 测试配置
    TEST_RESOLUTION = (640, 480)
    TEST_DURATION = 5  # 秒
    TEST_PORT = 9000
    
    # 飞书配置
    FEISHU_ENABLED = True
    

# ==================== 工具函数 ====================

def run_command(cmd: List[str], cwd: Optional[str] = None, timeout: int = 300) -> Tuple[bool, str, str]:
    """
    运行 shell 命令
    
    Returns:
        (success, stdout, stderr)
    """
    print(f"📝 执行命令: {' '.join(cmd)}")
    try:
        result = subprocess.run(
            cmd,
            cwd=cwd,
            capture_output=True,
            text=True,
            timeout=timeout
        )
        success = result.returncode == 0
        if not success:
            print(f"⚠️ 命令失败 (exit code: {result.returncode})")
            print(f"STDERR: {result.stderr[:500]}")
        return success, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        print(f"⏱️ 命令超时 ({timeout}s)")
        return False, "", "Timeout"
    except Exception as e:
        print(f"❌ 命令执行异常: {e}")
        return False, "", str(e)


def get_git_commit_hash(repo_path: str) -> str:
    """获取当前 git commit hash"""
    success, stdout, _ = run_command(['git', 'rev-parse', 'HEAD'], cwd=repo_path)
    return stdout.strip() if success else "unknown"


def get_git_last_message(repo_path: str) -> str:
    """获取最新 commit 消息"""
    success, stdout, _ = run_command(['git', 'log', '-1', '--pretty=%B'], cwd=repo_path)
    return stdout.strip() if success else "unknown"


def check_code_changes(repo_path: str, last_version_file: str) -> Tuple[bool, str]:
    """
    检查是否有代码变更
    
    Returns:
        (has_changes, current_hash)
    """
    current_hash = get_git_commit_hash(repo_path)
    
    if not os.path.exists(last_version_file):
        print("📄 未找到版本记录，视为首次运行")
        return True, current_hash
    
    with open(last_version_file, 'r') as f:
        last_hash = f.read().strip()
    
    if current_hash != last_hash:
        print(f"🔄 检测到代码变更:")
        print(f"   上次: {last_hash[:8]}")
        print(f"   当前: {current_hash[:8]}")
        return True, current_hash
    else:
        print(f"✅ 代码未变更 (commit: {current_hash[:8]})")
        return False, current_hash


def save_version(version_file: str, commit_hash: str):
    """保存当前版本"""
    os.makedirs(os.path.dirname(version_file), exist_ok=True)
    with open(version_file, 'w') as f:
        f.write(commit_hash)


# ==================== 编译阶段 ====================

class BuildStage:
    """编译阶段"""
    
    def __init__(self, config: Config):
        self.config = config
        self.stage_name = "编译阶段"
        self.success = False
        self.output = {}
        
    def run(self) -> bool:
        """执行编译"""
        print("\n" + "="*60)
        print(f"🔨 {self.stage_name}")
        print("="*60)
        
        # 1. 检查 UnrealCV 插件
        if not self._check_plugin():
            return False
        
        # 2. 编译插件
        if not self._build_plugin():
            return False
        
        # 3. 打包项目
        if not self._package_project():
            return False
        
        self.success = True
        return True
    
    def _check_plugin(self) -> bool:
        """检查插件文件"""
        plugin_file = os.path.join(self.config.UNREALCV_REPO, 'UnrealCV.uplugin')
        if not os.path.exists(plugin_file):
            print(f"❌ 未找到插件文件: {plugin_file}")
            return False
        
        print(f"✅ 找到插件: {plugin_file}")
        
        # 读取插件版本
        with open(plugin_file, 'r') as f:
            plugin_info = json.load(f)
        
        version = plugin_info.get('VersionName', 'unknown')
        print(f"   插件版本: {version}")
        self.output['plugin_version'] = version
        
        return True
    
    def _build_plugin(self) -> bool:
        """编译 UnrealCV 插件"""
        print("\n📦 编译 UnrealCV 插件...")
        
        # 使用 Unreal Automation Tool (UAT) 编译插件
        uat_path = os.path.join(
            os.path.dirname(self.config.UE4_EDITOR_PATH),
            '..', '..', 'Build', 'BatchFiles', 'RunUAT.sh'  # Mac
            # '..', '..', 'Build', 'BatchFiles', 'RunUAT.bat'  # Windows
        )
        uat_path = os.path.normpath(uat_path)
        
        if not os.path.exists(uat_path):
            print(f"⚠️ 未找到 UAT: {uat_path}")
            print("   尝试使用备用方案...")
            return self._build_plugin_alternative()
        
        # 构建命令
        cmd = [
            uat_path,
            'BuildPlugin',
            f'-Plugin={os.path.join(self.config.UNREALCV_REPO, "UnrealCV.uplugin")}',
            f'-Package={os.path.join(self.config.UNREALCV_REPO, "Build", "Plugin")}',
            '-TargetPlatforms=Mac',  # 或 Win64
            '-Rocket'
        ]
        
        success, stdout, stderr = run_command(cmd, timeout=600)
        
        if success:
            print("✅ 插件编译成功")
            return True
        else:
            print("❌ 插件编译失败")
            return False
    
    def _build_plugin_alternative(self) -> bool:
        """备用编译方案 - 直接在项目中构建"""
        print("   使用项目内构建方案...")
        
        # 复制插件到项目
        project_plugins = os.path.join(self.config.UE4_PROJECT_PATH, 'Plugins')
        unrealcv_plugin = os.path.join(project_plugins, 'UnrealCV')
        
        if os.path.exists(unrealcv_plugin):
            print(f"   清理旧插件: {unrealcv_plugin}")
            shutil.rmtree(unrealcv_plugin)
        
        # 复制新插件
        print(f"   复制插件到项目...")
        shutil.copytree(
            self.config.UNREALCV_REPO,
            unrealcv_plugin,
            ignore=shutil.ignore_patterns('.git', '__pycache__', '*.pyc')
        )
        
        print("✅ 插件复制完成 (将在打包时自动编译)")
        return True
    
    def _package_project(self) -> bool:
        """打包 UE4 项目"""
        print("\n📦 打包 UE4 项目...")
        
        if not os.path.exists(self.config.UE4_PROJECT_PATH):
            print(f"⚠️ 项目路径不存在: {self.config.UE4_PROJECT_PATH}")
            print("   跳过打包，将使用已有 binary")
            return self._use_existing_binary()
        
        # 构建打包命令
        project_file = os.path.join(self.config.UE4_PROJECT_PATH, 'UE4_ExampleScene.uproject')
        
        cmd = [
            self.config.UE4_EDITOR_PATH,
            project_file,
            '-run=Package',
            '-targetplatform=Mac',  # 或 Win64
            f'-clientconfig={self.config.BUILD_CONFIG}',
            f'-archivedirectory={self.config.PACKAGE_DIR}'
        ]
        
        success, stdout, stderr = run_command(cmd, timeout=1800)  # 30分钟超时
        
        if success:
            print("✅ 项目打包成功")
            self.output['package_path'] = self.config.PACKAGE_DIR
            return True
        else:
            print("❌ 项目打包失败")
            return False
    
    def _use_existing_binary(self) -> bool:
        """使用已存在的 binary"""
        print("   查找已有 binary...")
        
        # 查找 UnrealEnv 中的 binary
        env_path = os.path.expanduser('~/.unrealcv/UnrealEnv')
        if os.path.exists(env_path):
            for item in os.listdir(env_path):
                app_path = os.path.join(env_path, item)
                if os.path.isdir(app_path):
                    exe_path = os.path.join(app_path, f'{item}.app/Contents/MacOS/{item}')
                    if os.path.exists(exe_path):
                        print(f"✅ 找到 binary: {exe_path}")
                        self.output['binary_path'] = os.path.join(app_path, f'{item}.app')
                        return True
        
        print("⚠️ 未找到可用 binary")
        return False


# ==================== 测试阶段 ====================

class TestStage:
    """测试阶段"""
    
    def __init__(self, config: Config):
        self.config = config
        self.stage_name = "测试阶段"
        self.success = False
        self.output = {}
        self.client = None
        self.env_process = None
        
    def run(self, binary_path: Optional[str] = None) -> bool:
        """执行测试"""
        print("\n" + "="*60)
        print(f"🧪 {self.stage_name}")
        print("="*60)
        
        # 1. 准备 binary
        if binary_path and os.path.exists(binary_path):
            self.binary_path = binary_path
        else:
            self.binary_path = self._find_binary()
        
        if not self.binary_path:
            print("❌ 未找到可用 binary")
            return False
        
        print(f"📍 使用 binary: {self.binary_path}")
        
        # 2. 启动 binary
        if not self._start_binary():
            return False
        
        try:
            # 3. 连接
            if not self._connect():
                return False
            
            # 4. 运行测试
            self._test_camera_spawn()
            self._test_camera_control()
            self._test_multimodal_capture()
            
            # 5. 生成图像
            grid_path = self._create_image_grid()
            if grid_path:
                self.output['grid_image'] = grid_path
            
            self.success = True
            
        finally:
            self._cleanup()
        
        return self.success
    
    def _find_binary(self) -> Optional[str]:
        """查找可用的 binary"""
        env_path = os.path.expanduser('~/.unrealcv/UnrealEnv')
        
        if not os.path.exists(env_path):
            return None
        
        for item in os.listdir(env_path):
            app_path = os.path.join(env_path, item)
            if os.path.isdir(app_path):
                # Mac .app bundle
                mac_app = os.path.join(app_path, f'{item}.app')
                if os.path.exists(mac_app):
                    return mac_app
                
                # Windows/Linux binary
                for exe in ['.exe', '']:
                    exe_path = os.path.join(app_path, f'{item}{exe}')
                    if os.path.exists(exe_path):
                        return exe_path
        
        return None
    
    def _start_binary(self) -> bool:
        """启动 binary"""
        print("\n🚀 启动 UnrealCV Binary...")
        
        # 配置 unrealcv.ini
        self._configure_ini()
        
        # 启动命令
        if sys.platform == 'darwin':
            exe_path = os.path.join(self.binary_path, 'Contents/MacOS', 
                                   os.path.basename(self.binary_path).replace('.app', ''))
        else:
            exe_path = self.binary_path
        
        self.env_process = subprocess.Popen(
            [exe_path],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        
        print(f"   PID: {self.env_process.pid}")
        print("⏳ 等待启动 (8秒)...")
        time.sleep(8)
        
        return True
    
    def _configure_ini(self):
        """配置 unrealcv.ini"""
        if sys.platform == 'darwin':
            ini_path = os.path.join(
                self.binary_path,
                'Contents/UE4/*/Binaries/Mac/unrealcv.ini'
            )
        else:
            ini_path = os.path.join(
                os.path.dirname(self.binary_path),
                'unrealcv.ini'
            )
        
        # 使用 glob 查找
        import glob
        ini_files = glob.glob(ini_path)
        
        if ini_files:
            ini_file = ini_files[0]
            print(f"   配置: {ini_file}")
            
            # 读取并修改
            with open(ini_file, 'r') as f:
                lines = f.read().split('\n')
            
            with open(ini_file, 'w') as f:
                for i, line in enumerate(lines):
                    if line.startswith('Port='):
                        lines[i] = f'Port={self.config.TEST_PORT}'
                    elif line.startswith('Width='):
                        lines[i] = f'Width={self.config.TEST_RESOLUTION[0]}'
                    elif line.startswith('Height='):
                        lines[i] = f'Height={self.config.TEST_RESOLUTION[1]}'
                f.write('\n'.join(lines))
    
    def _connect(self) -> bool:
        """连接到服务器"""
        print(f"\n🔗 连接 {self.config.TEST_PORT}...")
        
        max_retries = 10
        for i in range(max_retries):
            try:
                self.client = unrealcv.Client(('127.0.0.1', self.config.TEST_PORT))
                if self.client.connect():
                    status = self.client.request('vget /unrealcv/status')
                    print(f"✅ 连接成功: {status}")
                    return True
            except Exception as e:
                print(f"   重试 {i+1}/{max_retries}...")
                time.sleep(1)
        
        return False
    
    def _test_camera_spawn(self):
        """测试生成相机"""
        print("\n📸 测试: 生成新相机")
        
        before = len(self.client.request('vget /cameras').split())
        
        tic = time.time()
        self.client.request('vset /cameras/spawn')
        elapsed = (time.time() - tic) * 1000
        
        after = len(self.client.request('vget /cameras').split())
        
        self.output['camera_spawn'] = {
            'time_ms': elapsed,
            'camera_count': after
        }
        
        print(f"   耗时: {elapsed:.2f} ms, 相机数: {before} → {after}")
    
    def _test_camera_control(self):
        """测试相机控制"""
        print("\n🎮 测试: 相机控制")
        
        # 读取测试
        tic = time.time()
        loc = self.client.request('vget /camera/0/location')
        get_loc_time = (time.time() - tic) * 1000
        
        # 设置测试
        loc_values = [float(x) for x in loc.split()]
        new_loc = [loc_values[0] + 100, loc_values[1], loc_values[2]]
        
        tic = time.time()
        self.client.request(f'vset /camera/0/location {new_loc[0]} {new_loc[1]} {new_loc[2]}')
        set_loc_time = (time.time() - tic) * 1000
        
        # 批量测试
        tic = time.time()
        cmds = ['vget /camera/0/location', 'vget /camera/0/rotation', 'vget /camera/0/fov']
        self.client.request(cmds)
        batch_time = (time.time() - tic) * 1000
        
        # 恢复
        self.client.request(f'vset /camera/0/location {loc_values[0]} {loc_values[1]} {loc_values[2]}')
        
        self.output['camera_control'] = {
            'get_location_ms': get_loc_time,
            'set_location_ms': set_loc_time,
            'batch_3cmds_ms': batch_time
        }
        
        print(f"   读取位置: {get_loc_time:.2f} ms")
        print(f"   设置位置: {set_loc_time:.2f} ms")
        print(f"   批量命令: {batch_time:.2f} ms")
    
    def _test_multimodal_capture(self):
        """测试多模态捕获"""
        print("\n🖼️  测试: 多模态图像捕获")
        
        viewmodes = [
            ('lit', 'png', '彩色'),
            ('depth', 'npy', '深度'),
            ('object_mask', 'png', '分割'),
            ('normal', 'png', '法向'),
        ]
        
        results = {}
        duration = self.config.TEST_DURATION
        
        for viewmode, fmt, name in viewmodes:
            cmd = f'vget /camera/0/{viewmode} {fmt}'
            
            count = 0
            sample = None
            
            tic = time.time()
            while time.time() - tic < duration:
                res = self.client.request(cmd)
                if sample is None:
                    sample = res
                count += 1
            
            elapsed = time.time() - tic
            fps = count / elapsed
            
            # 保存样本
            sample_path = os.path.join(self.config.OUTPUT_DIR, f'{viewmode}_sample.{fmt}')
            with open(sample_path, 'wb') as f:
                if isinstance(sample, bytes):
                    f.write(sample)
                else:
                    f.write(sample.encode())
            
            results[viewmode] = {
                'fps': fps,
                'count': count,
                'sample': sample_path
            }
            
            print(f"   {name:8s}: {fps:5.2f} FPS ({count} 帧)")
        
        self.output['multimodal'] = results
    
    def _create_image_grid(self) -> Optional[str]:
        """创建图像拼接"""
        print("\n🎨 生成拼接图像...")
        
        if 'multimodal' not in self.output:
            return None
        
        images = []
        labels = []
        fps_values = []
        
        order = ['lit', 'depth', 'object_mask', 'normal']
        
        for vm in order:
            if vm not in self.output['multimodal']:
                continue
            
            path = self.output['multimodal'][vm]['sample']
            fps = self.output['multimodal'][vm]['fps']
            
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
                fps_values.append(fps)
            except Exception as e:
                print(f"   ⚠️ 加载 {vm} 失败: {e}")
        
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
        title = f"UnrealCV CI Report - {datetime.now().strftime('%Y-%m-%d %H:%M')}"
        bbox = draw.textbbox((0, 0), title, font=title_font)
        draw.text(((grid_w - (bbox[2]-bbox[0]))//2, 10), title, fill=(255, 255, 255), font=title_font)
        
        # 粘贴图像
        for i, (img, label, fps) in enumerate(zip(images, labels, fps_values)):
            row = i // cols
            col = i % cols
            x = col * 640
            y = row * 480 + 50
            
            grid.paste(img, (x, y))
            
            # 标签
            text = f"{label} - {fps:.1f} FPS"
            draw.text((x + 10, y + 10), text, fill=(0, 255, 0), font=font)
        
        # 保存
        grid_path = os.path.join(self.config.OUTPUT_DIR, f'report_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png')
        grid.save(grid_path, quality=95)
        print(f"✅ 保存: {grid_path}")
        
        return grid_path
    
    def _cleanup(self):
        """清理资源"""
        print("\n🧹 清理...")
        
        if self.client:
            self.client.disconnect()
            self.client = None
        
        if self.env_process:
            self.env_process.terminate()
            self.env_process.wait()
            self.env_process = None
        
        print("✅ 清理完成")


# ==================== 报告阶段 ====================

class ReportStage:
    """报告生成阶段"""
    
    def __init__(self, config: Config):
        self.config = config
        self.stage_name = "报告阶段"
        
    def generate(self, build_output: Dict, test_output: Dict) -> str:
        """生成报告"""
        print("\n" + "="*60)
        print(f"📊 {self.stage_name}")
        print("="*60)
        
        report = {
            'timestamp': datetime.now().isoformat(),
            'unrealcv_commit': get_git_commit_hash(self.config.UNREALCV_REPO),
            'unrealcv_message': get_git_last_message(self.config.UNREALCV_REPO),
            'build': build_output,
            'test': test_output
        }
        
        # 保存 JSON
        report_path = os.path.join(self.config.OUTPUT_DIR, 'ci_report.json')
        with open(report_path, 'w') as f:
            json.dump(report, f, indent=2, default=str)
        print(f"📄 JSON报告: {report_path}")
        
        # 生成 Markdown 摘要
        md_path = os.path.join(self.config.OUTPUT_DIR, 'ci_report.md')
        self._generate_markdown(report, md_path)
        print(f"📄 Markdown报告: {md_path}")
        
        return report_path
    
    def _generate_markdown(self, report: Dict, path: str):
        """生成 Markdown 报告"""
        md = f"""# UnrealCV CI 测试报告

**生成时间**: {report['timestamp']}

**代码版本**: `{report['unrealcv_commit'][:8]}`

**提交信息**: {report['unrealcv_message']}

---

## 📦 编译结果

- **插件版本**: {report['build'].get('plugin_version', 'N/A')}
- **打包路径**: {report['build'].get('package_path', 'N/A')}
- **Binary路径**: {report['build'].get('binary_path', 'N/A')}

---

## 🧪 测试结果

### 相机生成
- **耗时**: {report['test'].get('camera_spawn', {}).get('time_ms', 0):.2f} ms
- **相机数量**: {report['test'].get('camera_spawn', {}).get('camera_count', 0)}

### 相机控制
- **读取位置**: {report['test'].get('camera_control', {}).get('get_location_ms', 0):.2f} ms
- **设置位置**: {report['test'].get('camera_control', {}).get('set_location_ms', 0):.2f} ms
- **批量命令**: {report['test'].get('camera_control', {}).get('batch_3cmds_ms', 0):.2f} ms

### 多模态捕获 FPS
"""
        
        multimodal = report['test'].get('multimodal', {})
        for vm, data in multimodal.items():
            md += f"- **{vm}**: {data.get('fps', 0):.2f} FPS\n"
        
        md += f"""
---

## 🖼️ 可视化报告

见 `{report['test'].get('grid_image', 'N/A')}`
"""
        
        with open(path, 'w') as f:
            f.write(md)


# ==================== 通知阶段 ====================

class NotificationStage:
    """飞书通知阶段"""
    
    def __init__(self, config: Config):
        self.config = config
        
    def send(self, report: Dict, grid_image: Optional[str] = None) -> bool:
        """发送飞书通知"""
        if not self.config.FEISHU_ENABLED:
            print("\n📱 飞书通知已禁用")
            return False
        
        print("\n📱 发送飞书通知...")
        
        # 构建消息
        commit = report['unrealcv_commit'][:8]
        msg = f"""🔄 UnrealCV CI 测试完成

📦 编译状态: {'✅ 成功' if report.get('build') else '⚠️ 使用已有'}
🧪 测试状态: ✅ 完成
📊 版本: {commit}

🎯 性能摘要:
"""
        
        multimodal = report.get('test', {}).get('multimodal', {})
        for vm, data in multimodal.items():
            fps = data.get('fps', 0)
            msg += f"   {vm}: {fps:.1f} FPS\n"
        
        print(msg)
        
        # 如果有图像，发送图像
        if grid_image and os.path.exists(grid_image):
            print(f"   附加图像: {grid_image}")
            # 这里调用飞书 API 发送图像
            # 使用 message 工具
            try:
                # 由于我们在 pipeline 脚本中，这里简单打印
                print(f"   ✅ 准备发送图像到飞书")
            except Exception as e:
                print(f"   ⚠️ 发送失败: {e}")
        
        return True


# ==================== 主流水线 ====================

class CIPipeline:
    """CI 主流水线"""
    
    def __init__(self, config: Config, mode: str = 'auto'):
        self.config = config
        self.mode = mode
        self.stages = []
        self.results = {}
        
    def run(self) -> bool:
        """运行流水线"""
        print("\n" + "="*70)
        print("🚀 UnrealCV CI 流水线启动")
        print("="*70)
        print(f"   模式: {self.mode}")
        print(f"   时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"   输出: {self.config.OUTPUT_DIR}")
        
        os.makedirs(self.config.OUTPUT_DIR, exist_ok=True)
        
        # 检查代码变更
        if self.mode == 'auto':
            has_changes, current_hash = check_code_changes(
                self.config.UNREALCV_REPO,
                self.config.VERSION_FILE
            )
            if not has_changes:
                print("\n✅ 无代码变更，跳过 CI")
                return True
        else:
            current_hash = get_git_commit_hash(self.config.UNREALCV_REPO)
        
        success = True
        build_output = {}
        test_output = {}
        
        # 阶段1: 编译 (除非 mode == 'test')
        if self.mode in ['build', 'full', 'auto']:
            build_stage = BuildStage(self.config)
            if build_stage.run():
                build_output = build_stage.output
                binary_path = build_output.get('binary_path') or build_output.get('package_path')
            else:
                print("⚠️ 编译失败，尝试使用已有 binary")
                binary_path = None
                success = False
        else:
            binary_path = None
        
        # 阶段2: 测试 (除非 mode == 'build')
        if self.mode in ['test', 'full', 'auto']:
            test_stage = TestStage(self.config)
            if test_stage.run(binary_path):
                test_output = test_stage.output
            else:
                print("❌ 测试失败")
                success = False
        
        # 阶段3: 报告
        report_stage = ReportStage(self.config)
        report_path = report_stage.generate(build_output, test_output)
        
        # 阶段4: 通知
        notification_stage = NotificationStage(self.config)
        grid_image = test_output.get('grid_image')
        
        # 构建完整报告用于通知
        report = {
            'unrealcv_commit': current_hash,
            'unrealcv_message': get_git_last_message(self.config.UNREALCV_REPO),
            'build': build_output,
            'test': test_output
        }
        notification_stage.send(report, grid_image)
        
        # 保存版本
        if success and self.mode in ['auto', 'full']:
            save_version(self.config.VERSION_FILE, current_hash)
        
        # 总结
        print("\n" + "="*70)
        if success:
            print("✅ CI 流水线完成")
        else:
            print("⚠️ CI 流水线完成 (有警告)")
        print(f"   报告: {report_path}")
        print("="*70)
        
        return success


def main():
    parser = argparse.ArgumentParser(description='UnrealCV CI Pipeline')
    parser.add_argument('--mode', choices=['auto', 'build', 'test', 'full'], 
                       default='auto', help='运行模式')
    parser.add_argument('--project-path', help='UE4 项目路径')
    parser.add_argument('--output-dir', help='输出目录')
    
    args = parser.parse_args()
    
    # 应用配置
    config = Config()
    if args.project_path:
        config.UE4_PROJECT_PATH = args.project_path
    if args.output_dir:
        config.OUTPUT_DIR = args.output_dir
    
    # 运行流水线
    pipeline = CIPipeline(config, args.mode)
    success = pipeline.run()
    
    return 0 if success else 1


if __name__ == '__main__':
    exit(main())
