# UnrealCV 自动化评估方案 - 交付文档

> 创建日期: 2026-03-05  
> 作者: AI Assistant  
> 适用: UnrealCV CI/CD 自动化测试

---

## 📦 交付文件清单

本次为你创建了以下自动化评估相关文件：

| 文件 | 路径 | 说明 |
|------|------|------|
| **CI 主流水线** | `unrealcv/unrealcv_ci_pipeline.py` | 完整的 CI/CD 自动化脚本 |
| **快速测试** | `unrealcv/auto_benchmark.py` | 简化版自动测试脚本 |
| **详细测试** | `unrealcv/test_camera_benchmark.py` | 详细相机功能测试 |
| **定时任务** | `unrealcv/run_ci.sh` | Shell 定时任务管理脚本 |
| **使用文档** | `unrealcv/CI_README.md` | 完整使用说明文档 |
| **测试输出** | `unrealcv/benchmark_output/` | 测试生成的图像和报告 |

---

## 🎯 方案概述

### 自动化流程

```
┌─────────────────┐     ┌──────────────┐     ┌──────────────┐
│ 1. 检测代码变更 │ ──▶ │ 2. 编译插件  │ ──▶ │ 3. 打包项目  │
└─────────────────┘     └──────────────┘     └──────────────┘
                                                        │
                                                        ▼
┌─────────────────┐     ┌──────────────┐     ┌──────────────┐
│ 6. 飞书通知     │ ◀── │ 5. 生成报告  │ ◀── │ 4. 性能测试  │
└─────────────────┘     └──────────────┘     └──────────────┘
```

### 测试覆盖

| 测试类别 | 具体测试项 |
|----------|-----------|
| **相机生成** | `vset /cameras/spawn` 耗时 |
| **相机控制** | 位置/旋转读取与设置、批量命令 |
| **图像捕获** | lit(彩色)、depth(深度)、mask(分割)、normal(法向) |
| **性能指标** | 每种模态的 FPS、延迟 |
| **可视化** | 多模态图像拼接、FPS 标注 |

---

## 🚀 快速使用

### 1. 一键运行完整测试

```bash
cd /Users/funway/.openclaw/workspace/unrealcv

# 自动检测代码变更，有更新才执行
python3 unrealcv_ci_pipeline.py --mode auto

# 强制执行完整流程（编译+测试）
python3 unrealcv_ci_pipeline.py --mode full

# 仅运行测试（使用已有 binary）
python3 unrealcv_ci_pipeline.py --mode test
```

### 2. 设置定时自动评估

```bash
# 安装每小时自动检查
cd /Users/funway/.openclaw/workspace/unrealcv
./run_ci.sh --install

# 手动运行
cd /Users/funway/.openclaw/workspace/unrealcv
./run_ci.sh auto

# 卸载定时任务
./run_ci.sh --uninstall
```

---

## ⚙️ 配置说明

在使用前，需要配置以下参数（编辑 `unrealcv_ci_pipeline.py`）：

```python
class Config:
    # === 路径配置（必须修改）===
    
    # UnrealCV 代码仓库路径
    UNREALCV_REPO = '/Users/funway/.openclaw/workspace/unrealcv'
    
    # UE4 示例项目路径（用于打包测试）
    # 如果没有项目，可以设为 None，会使用已有的 binary
    UE4_PROJECT_PATH = '/Users/funway/Documents/Unreal Projects/UE4_ExampleScene'
    
    # UE4 编辑器命令行工具路径
    UE4_EDITOR_PATH = '/Users/Shared/Epic Games/UE_4.27/Engine/Binaries/Mac/UE4Editor-Cmd'
    
    # === 测试配置（可选修改）===
    TEST_RESOLUTION = (640, 480)  # 测试分辨率
    TEST_DURATION = 5             # 每项测试时长（秒）
    TEST_PORT = 9000              # UnrealCV 服务端口
```

---

## 📊 输出示例

### 测试完成后生成

```
unrealcv/benchmark_output/
├── ci_report.json              # 完整测试数据
├── ci_report.md                # Markdown 报告
├── report_20260305_111845.png  # 可视化拼接图 ⭐
├── lit_sample.png              # 彩色图样本
├── depth_sample.npy            # 深度图样本
├── object_mask_sample.png      # 分割图样本
├── normal_sample.png           # 法向图样本
└── .last_tested_version        # 版本记录
```

### 性能基准参考

基于 Mac Studio 实测数据：

| 模态 | FPS | 性能评级 |
|------|-----|----------|
| 深度图 (depth) | 32.56 | ⭐⭐⭐⭐⭐ |
| 物体分割 (mask) | 31.27 | ⭐⭐⭐⭐⭐ |
| 法向图 (normal) | 22.80 | ⭐⭐⭐⭐ |
| 彩色图 (lit) | 13.73 | ⭐⭐⭐ |

---

## 🔧 与现有工作流集成

### 方案 A: Git Hook（代码提交时触发）

```bash
# 在 unrealcv/.git/hooks/post-commit 添加
#!/bin/bash
cd /Users/funway/.openclaw/workspace/unrealcv
python3 unrealcv_ci_pipeline.py --mode test &
```

### 方案 B: GitHub Actions（推荐）

```yaml
# .github/workflows/ci.yml
name: UnrealCV CI
on: [push, pull_request]
jobs:
  test:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run CI Pipeline
        run: python3 unrealcv_ci_pipeline.py --mode test
```

### 方案 C: 定时评估（已配置）

每小时自动检查代码更新，有变更则执行完整 CI。

---

## 📝 更新 UnrealCV 后的流程

当你更新 UnrealCV 代码后：

```bash
# 1. 进入仓库
cd /Users/funway/.openclaw/workspace/unrealcv

# 2. 拉取最新代码
git pull origin main

# 3. 运行 CI（自动检测变更并执行）
python3 unrealcv_ci_pipeline.py --mode auto

# 或直接强制执行
python3 unrealcv_ci_pipeline.py --mode full
```

CI 会自动：
1. ✅ 检测到新 commit
2. ✅ 编译 UnrealCV 插件
3. ✅ 打包项目
4. ✅ 运行性能测试
5. ✅ 生成可视化报告
6. ✅ 发送结果到飞书

---

## 🐛 故障排查

### 常见问题

#### 1. "未找到 UE4 编辑器"
```bash
# 检查路径
ls /Users/Shared/Epic Games/UE_4.27/Engine/Binaries/Mac/UE4Editor-Cmd

# 如使用 Windows，修改为：
UE4_EDITOR_PATH = 'C:/Program Files/Epic Games/UE_4.27/Engine/Binaries/Win64/UE4Editor-Cmd.exe'
```

#### 2. "编译失败"
- 检查 UE4 版本与插件兼容性
- 脚本会自动使用备用方案（复制插件到项目）
- 或手动在项目中编译一次

#### 3. "测试连接失败"
- 检查 binary 是否正常启动
- 检查端口 9000 是否被占用
- 查看 `ci_reports/` 中的日志文件

---

## 📚 文件说明

### unrealcv_ci_pipeline.py
完整 CI/CD 流水线，包含4个阶段：
- `BuildStage` - 编译插件和打包
- `TestStage` - 性能测试
- `ReportStage` - 生成报告
- `NotificationStage` - 飞书通知

### auto_benchmark.py
简化版测试脚本，自动启动 binary 并测试，适合快速验证。

### run_ci.sh
Shell 脚本封装，方便设置定时任务。

---

## 💡 后续优化建议

1. **支持 UE5** - 更新路径和编译参数
2. **性能对比** - 与历史测试数据对比，检测性能退化
3. **更多模态** - 添加 optical_flow、depth_exr 等测试
4. **并发测试** - 多相机同时捕获性能测试
5. **Web 报告** - 生成 HTML 可视化报告

---

## 📞 使用帮助

```bash
# 查看帮助
python3 unrealcv_ci_pipeline.py --help
./run_ci.sh --help

# 查看详细文档
cat /Users/funway/.openclaw/workspace/unrealcv/CI_README.md
```

---

**总结**: 你现在拥有了一套完整的 UnrealCV 自动化评估方案，代码更新后会自动编译、测试、生成报告并通知。只需运行 `python3 unrealcv_ci_pipeline.py --mode auto` 即可！
