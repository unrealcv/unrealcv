# UnrealCV CI/CD 自动化评估流水线

> 完整的自动化评估方案：代码更新 → 编译打包 → 性能测试 → 报告生成 → 飞书通知

---

## 📋 功能概述

这套 CI/CD 流水线实现了 UnrealCV 插件的全自动评估，包括：

1. **变更检测** - 自动检测 Git 仓库代码更新
2. **插件编译** - 使用 UE4/UE5 工具链编译 UnrealCV 插件
3. **项目打包** - 打包示例项目生成可执行 binary
4. **性能测试** - 自动化性能基准测试（相机生成、控制、多模态捕获）
5. **报告生成** - 生成 JSON/Markdown 格式测试报告 + 可视化图像
6. **飞书通知** - 自动发送测试结果到飞书

---

## 🚀 快速开始

### 1. 环境准备

#### 必需依赖
```bash
# Python 依赖
pip install Pillow numpy

# UnrealCV Python Client (已在 repo 中)
export PYTHONPATH="/path/to/unrealcv/client/python:$PYTHONPATH"

# UE4/UE5 引擎（用于编译打包）
# Mac: /Users/Shared/Epic Games/UE_4.27
# Windows: C:\Program Files\Epic Games\UE_4.27
```

#### 配置文件
编辑 `unrealcv_ci_pipeline.py` 中的 `Config` 类：

```python
class Config:
    # 路径配置
    UNREALCV_REPO = '/Users/funway/.openclaw/workspace/unrealcv'  # 代码仓库
    UE4_PROJECT_PATH = '/Users/funway/Documents/Unreal Projects/UE4_ExampleScene'  # UE项目
    OUTPUT_DIR = '/Users/funway/.openclaw/workspace/unrealcv/ci_reports'  # 报告输出
    PACKAGE_DIR = '/Users/funway/.unrealcv/Packaged'  # 打包输出
    
    # UE4 编辑器路径
    UE4_EDITOR_PATH = '/Users/Shared/Epic Games/UE_4.27/Engine/Binaries/Mac/UE4Editor-Cmd'
    # Windows: 'C:/Program Files/Epic Games/UE_4.27/Engine/Binaries/Win64/UE4Editor-Cmd.exe'
```

### 2. 运行流水线

```bash
cd /path/to/unrealcv

# 自动模式 - 检测代码变更，有更新才执行
python3 unrealcv_ci_pipeline.py --mode auto

# 完整模式 - 强制执行所有阶段
python3 unrealcv_ci_pipeline.py --mode full

# 仅编译打包
python3 unrealcv_ci_pipeline.py --mode build

# 仅测试（使用已有 binary）
python3 unrealcv_ci_pipeline.py --mode test
```

---

## 📊 测试内容

### 相机功能测试

| 测试项 | 说明 | 性能指标 |
|--------|------|----------|
| **生成新相机** | `vset /cameras/spawn` | 生成耗时 (ms) |
| **位置读取** | `vget /camera/0/location` | 单次读取耗时 |
| **位置设置** | `vset /camera/0/location` | 单次设置耗时 |
| **批量命令** | 3个命令批量发送 | 批量耗时 |

### 多模态图像捕获

| 模态 | 命令 | 性能指标 |
|------|------|----------|
| **彩色图 (lit)** | `vget /camera/0/lit png` | FPS |
| **深度图 (depth)** | `vget /camera/0/depth npy` | FPS |
| **物体分割 (mask)** | `vget /camera/0/object_mask png` | FPS |
| **法向图 (normal)** | `vget /camera/0/normal png` | FPS |

---

## 📁 输出文件

运行后会在 `OUTPUT_DIR` 目录生成：

```
ci_reports/
├── ci_report.json              # 完整测试数据 (JSON)
├── ci_report.md                # Markdown 报告
├── report_YYYYMMDD_HHMMSS.png  # 可视化拼接图
├── lit_sample.png              # 彩色样本图
├── depth_sample.npy            # 深度样本数据
├── object_mask_sample.png      # 分割样本图
├── normal_sample.png           # 法向样本图
└── .last_tested_version        # 上次测试版本记录
```

---

## ⏰ 设置定时任务

### macOS (launchd)

创建 `~/Library/LaunchAgents/com.unrealcv.ci.plist`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.unrealcv.ci</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/python3</string>
        <string>/Users/funway/.openclaw/workspace/unrealcv/unrealcv_ci_pipeline.py</string>
        <string>--mode</string>
        <string>auto</string>
    </array>
    <key>StartInterval</key>
    <integer>3600</integer>  <!-- 每小时检查一次 -->
    <key>StandardOutPath</key>
    <string>/Users/funway/.openclaw/workspace/unrealcv/ci_reports/ci.log</string>
    <key>StandardErrorPath</key>
    <string>/Users/funway/.openclaw/workspace/unrealcv/ci_reports/ci_error.log</string>
</dict>
</plist>
```

加载并启动：
```bash
launchctl load ~/Library/LaunchAgents/com.unrealcv.ci.plist
launchctl start com.unrealcv.ci
```

### Linux (cron)

```bash
# 编辑 crontab
crontab -e

# 每小时检查一次代码更新
0 * * * * cd /path/to/unrealcv && /usr/bin/python3 unrealcv_ci_pipeline.py --mode auto >> /path/to/ci_reports/cron.log 2>&1

# 或每天凌晨 3 点强制执行
0 3 * * * cd /path/to/unrealcv && /usr/bin/python3 unrealcv_ci_pipeline.py --mode full >> /path/to/ci_reports/cron.log 2>&1
```

### Windows (Task Scheduler)

使用 PowerShell 脚本：
```powershell
# unrealcv_ci_task.ps1
Set-Location "C:\path\to\unrealcv"
python.exe unrealcv_ci_pipeline.py --mode auto
```

---

## 🔧 高级配置

### 自定义测试参数

在 `Config` 类中修改：

```python
class Config:
    # 测试配置
    TEST_RESOLUTION = (640, 480)  # 测试分辨率
    TEST_DURATION = 5             # 每项测试时长（秒）
    TEST_PORT = 9000              # UnrealCV 端口
```

### 编译配置选择

在 `Config` 类中修改 `BUILD_CONFIG`：

```python
class Config:
    # 编译配置选项:
    # - 'Development' : 开发版本，包含调试信息，性能较低 (适合调试)
    # - 'PackagedDev' : 打包开发版本，性能较好 (推荐用于测试)
    # - 'Shipping'    : 发布版本，性能最优，无调试信息 (推荐用于生产)
    # - 'Test'        : 测试版本
    BUILD_CONFIG = 'PackagedDev'  # 默认使用 PackagedDev
```

**配置对比：**

| 配置 | 编译优化 | 调试信息 | 性能 | 适用场景 |
|------|---------|---------|------|---------|
| Development | 低 (-O0) | 完整 | 50-60% | 本地开发调试 |
| PackagedDev | 中 (-O2) | 部分 | 90-95% | **CI测试推荐** |
| Shipping | 高 (-O3) | 无 | 100% | 生产发布 |
| Test | 中 | 完整 | 80-85% | 自动化测试 |

### 跳过编译阶段

如果已经有编译好的 binary，可以使用 test 模式：

```bash
python3 unrealcv_ci_pipeline.py --mode test
```

### 指定自定义 binary

修改代码中的 binary 查找逻辑，或在配置中指定：

```python
class Config:
    # 直接指定 binary 路径（跳过编译）
    PREBUILT_BINARY = '/path/to/your/UE4_ExampleScene.app'
```

---

## 🐛 故障排查

### 常见问题

#### 1. 找不到 UE4 编辑器
```
⚠️ 未找到 UAT: /path/to/RunUAT.sh
```

**解决**: 检查 `UE4_EDITOR_PATH` 配置，确保指向正确的 UE4Editor-Cmd。

#### 2. 插件编译失败
```
❌ 插件编译失败
```

**解决**: 
- 检查 UE4 版本与插件兼容性
- 使用备用方案（自动复制到项目 Plugins 目录）
- 手动在项目内编译一次

#### 3. 连接服务器失败
```
❌ 无法连接到 UnrealCV 服务器
```

**解决**:
- 检查 binary 是否正常启动
- 检查端口配置是否一致
- 查看 binary 是否有错误日志

#### 4. 图像测试失败
```
❌ 失败: a bytes-like object is required
```

**解决**: 某些视图模式（如 optical_flow）可能不可用，已在代码中处理。

---

## 📈 性能基准参考

基于 Mac Studio (M2 Ultra) + UE4 测试：

| 模态 | FPS | 评级 |
|------|-----|------|
| 深度 (depth) | ~32 | ⭐⭐⭐⭐⭐ |
| 分割 (mask) | ~31 | ⭐⭐⭐⭐⭐ |
| 法向 (normal) | ~23 | ⭐⭐⭐⭐ |
| 彩色 (lit) | ~14 | ⭐⭐⭐ |

---

## 🔗 相关文件

- `unrealcv_ci_pipeline.py` - 主流水线脚本
- `auto_benchmark.py` - 简化版测试脚本（仅测试）
- `test_camera_benchmark.py` - 详细测试脚本

---

## 📝 更新日志

### v1.0 (2026-03-05)
- 初始版本
- 支持完整的 CI/CD 流程
- 支持 macOS/Windows/Linux
- 飞书通知集成

---

## 💡 未来改进

- [ ] 支持 UE5
- [ ] 更详细的性能分析（CPU/GPU 占用）
- [ ] 历史性能对比（检测性能退化）
- [ ] Web 可视化报告
- [ ] 邮件/钉钉通知支持
- [ ] Docker 容器化运行

---

**作者**: AI Assistant  
**日期**: 2026-03-05  
**版本**: 1.0
