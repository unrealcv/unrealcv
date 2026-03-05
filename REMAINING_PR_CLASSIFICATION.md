# UnrealCV 剩余 PR 分类报告

**生成时间**: 2026-03-05  
**已合并**: 13 个 PR  
**剩余**: 29 个 PR

---

## 📊 已合并 PR 列表（今日完成）

### Code Quality (8个)
- ~~#324~~ - C++ server quality hardening ✅
- ~~#322~~ - Structured logging ✅
- ~~#298~~ - Example portability ✅
- ~~#300~~ - UE5 build hygiene ✅
- ~~#305~~ - C++ object lifetimes ✅
- ~~#313~~ - Remove Python 2 compat ✅
- ~~#307~~ - Python runtime hardening ✅
- ~~#296~~ - Python client refactoring ✅

### Documentation (2个)
- ~~#325~~ - Sphinx warning fixes ✅
- ~~#297~~ - UE5 docs modernization ✅

### CI/Scripts (2个)
- ~~#302~~ - Release scripts modernization ✅
- ~~#303~~ - CI workflow ✅

---

## 📋 剩余 PR 分类（29个）

### 🔥 Critical/Bug Fixes (高优先级 - 4个)

| # | 标题 | 解决的问题 | 风险 |
|---|------|-----------|------|
| **321** | fix: resource leaks, mutable defaults, shell injection | 资源泄漏、shell注入漏洞、可变默认参数 | 高 |
| **319** | fix: thread-safe Client with socket locking | 线程安全问题、Socket锁定 | 中 |
| **304** | Fix critical C++ command logic and null-safety | C++命令逻辑错误、空指针安全 | 高 |
| **294** | Fix critical Python runtime bugs | Python运行时关键Bug | 高 |

**建议**: 立即合并，涉及安全和稳定性

---

### ⚡ Performance (性能优化 - 3个)

| # | 标题 | 解决的问题 | 预期收益 |
|---|------|-----------|----------|
| **320** | perf: optimise image decode pipeline | 图像解码性能、修复np.float弃用 | 内存优化 |
| **318** | perf: zero-copy socket I/O | 零拷贝Socket I/O、预分配缓冲区 | **5.6x速度提升** |
| **308** | Optimize C++ hot paths | Actor查找、序列化、IO循环优化 | 响应速度 |

**建议**: 高优先级，显著提升性能

---

### 🔒 Security (安全 - 1个)

| # | 标题 | 解决的问题 |
|---|------|-----------|
| **299** | Secure server defaults | localhost绑定、可选认证、命令策略 |

**建议**: 高优先级，生产环境必需

---

### 🧪 Testing (测试增强 - 3个)

| # | 标题 | 解决的问题 |
|---|------|-----------|
| **323** | test(client): migration-hardening non-regression | 非回归测试套件 |
| **317** | test: add 34 pure unit tests | 纯单元测试(工具、SocketMessage、MsgDecoder) |
| **301** | Add command contract tests | 命令契约测试、协议稳定性 |

**建议**: 中优先级，提升代码质量

---

### 🚀 Features (新功能 - 4个)

| # | 标题 | 解决的问题 |
|---|------|-----------|
| **316** | feat: add Pydantic v2 data models | 结构化API类型数据模型 |
| **315** | feat: add structured exception hierarchy | 结构化异常层次、客户端上下文管理器 |
| **314** | feat: add comprehensive type annotations | Python客户端完整类型注解 |
| **312** | Add Python public API snapshot | API快照和兼容性保护 |

**建议**: 低优先级，增强功能非必需

---

### 🛠️ Tooling (工具 - 3个)

| # | 标题 | 解决的问题 |
|---|------|-----------|
| **311** | Add generated command schema | 命令schema生成和漂移验证工具 |
| **310** | Add clang-format and clang-tidy | C++代码质量门禁 |
| **306** | Add Python quality gates | Python质量门禁(Ruff, Mypy, pre-commit) |

**建议**: 中优先级，提升代码规范

---

### 📦 CI/CD (1个)

| # | 标题 | 解决的问题 |
|---|------|-----------|
| **295** | Add GitHub Actions CI | GitHub Actions CI和tox配置现代化 |

**建议**: 中优先级，自动化测试

---

### ⏰ Legacy/Outdated (过时 - 8个)

| # | 标题 | 状态 |
|---|------|------|
| **205** | Adapted to UE4.26.0 | ⚠️ 过时 (UE4) |
| **185** | Adapted to UE4.24.3 | ⚠️ 过时 (UE4) |
| **140** | Add Unreal 4.19 | ⚠️ 过时 (UE4) |
| **139** | Implemented vset /action/game/resume | 🟡 可能有用 |
| **131** | Python build automation path | 🟡 可能有用 |
| **69** | Fixed GetConsole()->Log issue | 🟡 可能有用 |

**建议**: 跳过，当前项目已聚焦UE5

---

## 🎯 推荐合并顺序

### 第一阶段：Critical + Security (明天)
1. **#321** - 资源泄漏、shell注入修复 ⭐⭐⭐
2. **#319** - 线程安全、Socket锁定 ⭐⭐⭐
3. **#304** - C++命令逻辑修复 ⭐⭐⭐
4. **#294** - Python运行时关键Bug ⭐⭐⭐
5. **#299** - 安全默认配置 ⭐⭐⭐

### 第二阶段：Performance (后天)
6. **#318** - 零拷贝Socket I/O ⭐⭐⭐ (5.6x性能提升!)
7. **#320** - 图像解码优化 ⭐⭐
8. **#308** - C++热路径优化 ⭐⭐

### 第三阶段：Testing (下周)
9. **#323** - 非回归测试套件 ⭐⭐
10. **#317** - 34个单元测试 ⭐⭐
11. **#301** - 命令契约测试 ⭐⭐

### 第四阶段：Tooling (后续)
12. **#310** - C++质量门禁 ⭐
13. **#306** - Python质量门禁 ⭐
14. **#295** - GitHub Actions CI ⭐

### 第五阶段：Features (按需)
15. **#314** - 类型注解
16. **#315** - 结构化异常
17. **#316** - Pydantic模型
18. **#312** - API兼容性保护

---

## 📊 统计

| 类别 | 数量 | 建议优先级 |
|------|------|-----------|
| 🔥 Critical/Bug | 4 | 立即 |
| ⚡ Performance | 3 | 明天 |
| 🔒 Security | 1 | 立即 |
| 🧪 Testing | 3 | 下周 |
| 🛠️ Tooling | 3 | 后续 |
| 📦 CI/CD | 1 | 后续 |
| 🚀 Features | 4 | 按需 |
| ⏰ Legacy | 7 | 跳过 |

---

**建议明天开始合并 Critical + Security + Performance PR！**