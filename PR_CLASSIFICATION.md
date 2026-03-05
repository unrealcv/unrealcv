# UnrealCV PR 分类报告

> 生成时间: 2026-03-05
> 总计: 37 个未合并 PR

---

## 📊 PR 分类概览

### 按作者分类
| 作者 | PR 数量 | 主要领域 |
|------|---------|----------|
| **vlordier** | 30 | 代码质量、性能、测试、CI/CD |
| 其他 | 7 | UE 版本适配、功能修复 |

### 按类型分类
| 类型 | 数量 | 优先级 |
|------|------|--------|
| 🔥 **Critical/Bug Fixes** | 4 | 高 |
| ⚡ **Performance** | 3 | 高 |
| 🔒 **Security** | 1 | 高 |
| 🧪 **Testing** | 3 | 中 |
| 🛠️ **Code Quality** | 10 | 中 |
| 📦 **CI/CD & Tooling** | 7 | 中 |
| 📝 **Documentation** | 2 | 低 |
| 🚀 **Features** | 3 | 低 |
| ⏰ **Legacy (旧版本适配)** | 4 | 低/过时 |

---

## 🔥 Critical/Bug Fixes (高优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **321** | fix: resource leaks, mutable defaults, shell injection, and bugs | vlordier | 修复资源泄漏、shell注入漏洞 |
| **319** | fix: thread-safe Client with socket locking and robust reconnect | vlordier | 线程安全、Socket锁定 |
| **304** | Fix critical C++ command logic and null-safety regressions | vlordier | C++ 命令逻辑、空安全 |
| **294** | Fix critical Python runtime bugs and broken examples | vlordier | Python 运行时关键Bug |

---

## ⚡ Performance (高优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **320** | perf: optimise image decode pipeline, fix np.float deprecation | vlordier | 图像解码优化 |
| **318** | perf: zero-copy socket I/O with pre-allocated buffers | vlordier | **零拷贝Socket I/O** |
| **308** | Optimize C++ hot paths for actor lookup, serialization, and IO loops | vlordier | C++ 热路径优化 |

---

## 🔒 Security (高优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **299** | Secure server defaults: localhost bind, optional auth, command policy | vlordier | 默认安全：本地绑定、认证 |

---

## 🧪 Testing (中优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **323** | test(client): migration-hardening non-regression suite | vlordier | 非回归测试套件 |
| **317** | test: add 34 pure unit tests for util, SocketMessage, and MsgDecoder | vlordier | 34个单元测试 |
| **301** | Add command contract tests for server protocol stability | vlordier | 命令契约测试 |

---

## 🛠️ Code Quality/Refactoring (中优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **324** | refactor(server): C++ server quality hardening (rounds 1-4) | vlordier | C++ 服务器代码加固 |
| **322** | refactor: replace print statements with structured logging | vlordier | 结构化日志 |
| **313** | fix: Remove Python 2 compat, fix bugs, modernize idioms | vlordier | 移除 Python 2 兼容 |
| **307** | Harden Python client runtime: safer errors, parsing, and process execution | vlordier | Python 运行时加固 |
| **305** | Harden C++ object lifetimes and ownership in UnrealCV server | vlordier | C++ 对象生命周期 |
| **296** | Refactor Python client exports, typing, and API resilience | vlordier | Python 重构 |
| **300** | UE5 build hygiene: modern includes and IWYU enforcement | vlordier | UE5 构建清理 |
| **298** | Make data_generation example portable | vlordier | 示例可移植性 |
| **69** | (fix) Fixed an issue with GetConsole()->Log... | zumlernen-de | 控制台日志修复 |
| **139** | Implemented vset /action/game/resume | martijn-vinotion | 恢复游戏动作 |

---

## 📦 CI/CD & Tooling (中优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **310** | Add clang-format and clang-tidy quality gates for C++ PRs | vlordier | C++ 代码质量门禁 |
| **309** | Add one-command dev environment with devcontainer and tasks | vlordier | 开发环境 |
| **306** | Add Python quality gates with Ruff, Mypy, pre-commit and CI | vlordier | Python 质量门禁 |
| **303** | Add release-maintenance CI workflow for tooling validation | vlordier | 发布维护工作流 |
| **302** | Modernize release scripts for portability and CI usage | vlordier | 发布脚本现代化 |
| **295** | Add GitHub Actions CI and modernize tox configuration | vlordier | GitHub Actions CI |
| **131** | Make python build automation consider /Users/Shared/EpicGames/ for engine path | analog-garage | 构建路径适配 |

---

## 📝 Documentation (低优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **325** | docs: reduce Sphinx warning noise and fix docs toctree consistency | vlordier | Sphinx 文档修复 |
| **297** | Modernize docs for Python 3, UE5, and current Sphinx | vlordier | 文档现代化 |

---

## 🚀 Features (低优先级)

| # | 标题 | 作者 | 说明 |
|---|------|------|------|
| **316** | feat: add Pydantic v2 data models for structured API types | vlordier | Pydantic v2 模型 |
| **315** | feat: add structured exception hierarchy and Client context manager | vlordier | 结构化异常 |
| **314** | feat: add comprehensive type annotations across Python client | vlordier | Python 类型注解 |
| **312** | Add Python public API snapshot and compatibility guard | vlordier | API 兼容性保护 |
| **311** | Add generated command schema and drift validation tooling | vlordier | 命令 schema 验证 |

---

## ⏰ Legacy/Outdated (低优先级/过时)

| # | 标题 | 作者 | 说明 | 状态 |
|---|------|------|------|------|
| **205** | Adapted to UE4.26.0 | HamiltonHuaji | UE 4.26 适配 | ⚠️ 过时 |
| **185** | Adapted to new API (builds with UE4.24.3) | nathanael-k | UE 4.24 适配 | ⚠️ 过时 |
| **140** | .Add Unreal 4.19 | kobewangSky | UE 4.19 适配 | ⚠️ 过时 |

---

## 📋 建议合并顺序

### 第一阶段: Critical + Performance + Security (本周)
1. #321 - 资源泄漏、shell注入修复
2. #319 - 线程安全、Socket锁定
3. #304 - C++ 命令逻辑修复
4. #294 - Python 运行时关键Bug
5. #318 - 零拷贝Socket I/O (性能)
6. #299 - 安全默认配置

### 第二阶段: Testing + CI/CD (下周)
7. #317 - 34个单元测试
8. #323 - 非回归测试套件
9. #306 - Python 质量门禁
10. #310 - C++ 质量门禁
11. #295 - GitHub Actions CI

### 第三阶段: Code Quality (后续)
12. #313 - 移除 Python 2 兼容
13. #322 - 结构化日志
14. #305 - C++ 生命周期加固
15. #307 - Python 运行时加固

### 第四阶段: Features + Docs (按需)
16. #314 - Python 类型注解
17. #315 - 结构化异常
18. #325 - 文档修复

---

## 🚫 不建议合并

- **#205, #185, #140** - UE4 版本适配，当前项目已聚焦 UE5

---

*报告生成时间: 2026-03-05*
