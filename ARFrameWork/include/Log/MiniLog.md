# OpenAR 框架 `ar::info()` 函数详解

## 函数签名与定义

```cpp
// 头文件：ARFrameWork/include/Log/MiniLog.h
namespace ar {

template <typename... _Types>
void info(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
    std::string msg = std::vformat(_Fmt.get(), std::make_format_args(_Args...));
    detail::callMiniLog(msg, level::info);
}

}
```

## 核心功能

**作用**：类型安全的格式化日志输出函数，封装 spdlog 库，支持类似 Python `print()` 的格式化字符串语法。

**设计目标**：
1. ✅ 简洁的 API（`ar::info("x: {}, y: {}", x, y)`）
2. ✅ 编译期类型检查（错误的格式化参数会编译失败）
3. ✅ 零运行时开销（模板展开 + 内联优化）
4. ✅ 统一的日志级别管理

---

## C++ 核心语法解析

### 1. 可变参数模板（Variadic Templates，C++11）

```cpp
template <typename... _Types>
//                ^^^ 参数包声明（0 个或多个类型）
```

**作用**：允许函数接受任意数量、任意类型的参数。

#### 示例：参数包展开

```cpp
// 基础示例：打印任意数量参数
template <typename... Args>
void print(Args... args) {
    (std::cout << ... << args);  // C++17 折叠表达式
}

print(1, "hello", 3.14);  // 输出：1hello3.14

// OpenAR 中的实际用法
ar::info("x: {}, y: {}", 100, 200);
// _Types 被推导为 <int, int>
// _Args 被推导为 {100, 200}
```

#### 参数包的三种操作

```cpp
template <typename... Args>
void foo(Args... args) {
    // 1. 获取参数数量
    size_t count = sizeof...(Args);  // 编译期常量
    
    // 2. 展开参数包（C++17 折叠表达式）
    ((std::cout << args << " "), ...);
    
    // 3. 转发参数包
    bar(std::forward<Args>(args)...);
}
```

---

### 2. 完美转发（Perfect Forwarding，C++11）

```cpp
void info(const std::format_string<_Types...> _Fmt, _Types&&... _Args)
//                                                      ^^^ 万能引用（forwarding reference）
```

#### 万能引用的类型推导规则

```cpp
template <typename T>
void func(T&& arg);

int x = 42;
func(x);       // T 推导为 int&，arg 类型为 int&
func(10);      // T 推导为 int，arg 类型为 int&&

// 在 ar::info 中
ar::info("value: {}", x);       // _Args = int&
ar::info("value: {}", 42);      // _Args = int
ar::info("value: {}", std::move(str));  // _Args = std::string&&
```

#### 为什么使用 `&&`？

```cpp
// ❌ 值传递：会拷贝临时对象
template <typename... Args>
void log(Args... args);
log(std::string("hello"));  // 拷贝构造 std::string

// ✅ 万能引用：避免不必要的拷贝
template <typename... Args>
void log(Args&&... args);
log(std::string("hello"));  // 移动语义，零拷贝
```

---

### 3. `std::format_string`（C++20 格式化字符串）

```cpp
const std::format_string<_Types...> _Fmt
```

#### 类型安全的编译期检查

```cpp
// ✅ 正确：格式化参数匹配
ar::info("x: {}, y: {}", 10, 20);  // 编译通过

// ❌ 错误：参数数量不匹配
ar::info("x: {}, y: {}", 10);  // 编译错误：缺少参数

// ❌ 错误：类型不兼容
struct Point { int x, y; };
Point p{1, 2};
ar::info("point: {}", p);  // 编译错误：Point 不可格式化
```

#### 与 `printf` 对比

```cpp
// ❌ printf：运行时检查，类型不安全
printf("value: %s\n", 42);  // 编译通过，运行时崩溃！

// ✅ std::format：编译期检查，类型安全
ar::info("value: {}", 42);  // ✅ 编译通过
ar::info("value: {}", Point{1, 2});  // ❌ 编译错误
```

---

### 4. 格式化字符串处理

```cpp
std::string msg = std::vformat(_Fmt.get(), std::make_format_args(_Args...));
```

#### 步骤拆解

```cpp
// 1. _Fmt.get() 获取底层格式化字符串
std::format_string<int, int> fmt = "x: {}, y: {}";
const char* raw_fmt = fmt.get();  // "x: {}, y: {}"

// 2. std::make_format_args 创建参数存储
auto args = std::make_format_args(10, 20);
// 内部存储类型擦除的参数引用

// 3. std::vformat 格式化字符串
std::string result = std::vformat(raw_fmt, args);
// result = "x: 10, y: 20"
```

#### 为什么用 `std::vformat` 而非 `std::format`？

```cpp
// std::format：直接接受参数包（模板函数）
template <typename... Args>
std::string format(std::format_string<Args...> fmt, Args&&... args);

// std::vformat：接受类型擦除的参数（非模板函数）
std::string vformat(std::string_view fmt, std::format_args args);

// OpenAR 的设计：
// 模板部分（编译期）：ar::info()
// 非模板部分（运行时）：detail::callMiniLog() → spdlog
```

**优势**：
- ✅ 减少模板实例化（`detail::callMiniLog` 不是模板）
- ✅ 更小的二进制体积（只有一个 `callMiniLog` 符号）
- ✅ 简化链接器工作

---

## 在 OpenAR 中的实际应用

### 应用场景 1：任务执行日志

```cpp
// ARLauncher/src/ARLTaskNode.cpp:32
void ARLTaskNode::play()
{
    ar::info("Task Name : {} Task ID : {}", task_data->task_name, task_data->task_id);
    // 输出：Task Name : MainTask Task ID : 1
    
    for (int i = 0; i < task_data->Size(); i++) {
        if (is_return) return;
        ARBlock* block_ptr = task_data->getPtr(i);
        playPerLoopingBlock(block_ptr);
    }
}
```

### 应用场景 2：设备控制日志

```cpp
// ARFrameWork/src/DeviceController/MuMuDeviceController.cpp:143
ar::ARDeviceError ar::MuMuDeviceController::lauchDevice(const int& index) {
    // ...执行启动命令...
    
    ar::info("Launch MuMu {}", index);
    // 输出：Launch MuMu 1
    
    return ar::ARDeviceError::AR_NO_ERROR;
}
```

### 应用场景 3：性能测试日志

```cpp
// PCRDemo/src/Init.cpp:65-70
auto start = std::chrono::high_resolution_clock::now();
ar::point p = image_recognition->compareImageReturnCentrePoint(image, temp, 0.95f);
auto end = std::chrono::high_resolution_clock::now();

if (!p.is_empty) {
    ar::info("x: {} y: {}", p.x, p.y);
    // 输出：x: 100 y: 200
}

std::chrono::duration<double, std::milli> duration = end - start;
ar::info("MPR Match two image use {} ms !", (int)duration.count());
// 输出：MPR Match two image use 15 ms !
```

### 应用场景 4：错误处理

```cpp
// ARFrameWork/src/DeviceController/MuMuDeviceController.cpp:39
if (info.error_code == -200) {
    ar::error("Invalid MuMu index: {}", index);
    // 输出：[error] Invalid MuMu index: 10
    return ar::ARDeviceError::AR_INVALID_INDEX;
}
```

---

## 日志系统的完整调用链

```
用户代码
    ↓
ar::info("x: {}, y: {}", x, y)  ← 模板函数（编译期展开）
    ↓
std::vformat(_Fmt.get(), std::make_format_args(_Args...))  ← 格式化字符串
    ↓
detail::callMiniLog(msg, level::info)  ← 非模板函数（统一接口）
    ↓
spdlog::info(msg)  ← 第三方日志库
    ↓
控制台/文件输出
```

### `detail::callMiniLog` 的实现

```cpp
// ARFrameWork/src/Log/MiniLog.cpp:76-83
void ar::detail::callMiniLog(const std::string& msg, ar::level lv) {
    if (lv == ar::level::trace) spdlog::trace(msg);
    if (lv == ar::level::debug) spdlog::debug(msg);
    if (lv == ar::level::info) spdlog::info(msg);      // ← 这里被调用
    if (lv == ar::level::warn) spdlog::warn(msg);
    if (lv == ar::level::err) spdlog::error(msg);
    if (lv == ar::level::critical) spdlog::critical(msg);
}
```

---

## 支持的格式化语法

### 基础占位符

```cpp
ar::info("value: {}", 42);           // value: 42
ar::info("{} + {} = {}", 1, 2, 3);   // 1 + 2 = 3
```

### 位置参数

```cpp
ar::info("{1} {0}", "world", "hello");  // hello world
ar::info("{0} {0} {1}", "a", "b");      // a a b
```

### 格式化选项

```cpp
ar::info("hex: {:#x}", 255);           // hex: 0xff
ar::info("float: {:.2f}", 3.14159);    // float: 3.14
ar::info("pad: {:>5}", 42);            // pad:    42
ar::info("fill: {:*<5}", 42);          // fill: 42***
```

### 自定义类型（需要特化 `std::formatter`）

```cpp
// 为 ar::point 添加格式化支持
template <>
struct std::formatter<ar::point> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    
    auto format(const ar::point& p, format_context& ctx) const {
        return format_to(ctx.out(), "({}, {})", p.x, p.y);
    }
};

// 现在可以直接格式化 ar::point
ar::point p{100, 200};
ar::info("point: {}", p);  // point: (100, 200)
```

---

## 性能分析

### 编译期优化

```cpp
// 原始代码
ar::info("x: {}, y: {}", 10, 20);

// 编译器展开后（等价）
{
    std::format_string<int, int> _Fmt = "x: {}, y: {}";
    int _Arg0 = 10;
    int _Arg1 = 20;
    
    std::string msg = std::vformat(_Fmt.get(), std::make_format_args(_Arg0, _Arg1));
    ar::detail::callMiniLog(msg, ar::level::info);
}

// Release 模式优化后（内联展开）
{
    std::string msg = "x: 10, y: 20";  // 编译期计算（如果可能）
    spdlog::info(msg);                 // 直接调用
}
```

### 与 `printf` 性能对比

| 特性 | `printf` | `ar::info` |
|------|----------|-----------|
| **类型安全** | ❌ 运行时检查 | ✅ 编译期检查 |
| **格式化性能** | 快（C 风格） | 中等（C++ 风格） |
| **线程安全** | ❌ 需要加锁 | ✅ spdlog 内置 |
| **可扩展性** | ❌ 固定格式 | ✅ 自定义类型 |
| **二进制大小** | 小 | 中等（模板实例化） |

---

## 日志级别控制

### 设置日志级别

```cpp
// PCRDemo/src/main.cpp:15
ar::setMiniLogLevel(ar::level::debug);  // 显示 debug 及以上级别

// 现在只输出 info/warn/error/critical
ar::debug("This is hidden");  // ❌ 不输出
ar::info("This is shown");    // ✅ 输出
```

### 日志级别优先级

```cpp
namespace ar {
    enum class level {
        trace,      // 0：最详细
        debug,      // 1：调试信息
        info,       // 2：一般信息（默认）
        warn,       // 3：警告
        err,        // 4：错误
        critical,   // 5：致命错误
    };
}
```

### 输出配置

```cpp
// 1. 控制台输出（默认）
ar::miniLogInit(ar::level::debug);

// 2. 文件输出
ar::setMiniLogFileSink("app.log", ar::level::info, 1000);

// 3. 多输出（控制台 + 文件）
ar::setMiniLogMultiSinks("app.log", ar::level::debug, 1000);
```

---

## 与其他日志函数对比

| 函数 | 级别 | 用途 | 示例 |
|------|------|------|------|
| `ar::trace()` | 最低 | 详细追踪信息 | `ar::trace("Enter function: {}", __func__)` |
| `ar::debug()` | 低 | 调试信息 | `ar::debug("Variable x = {}", x)` |
| **`ar::info()`** | **中** | **一般信息** | `ar::info("Task started")` |
| `ar::warn()` | 高 | 警告信息 | `ar::warn("num_points < 8 not recommended")` |
| `ar::error()` | 很高 | 错误信息 | `ar::error("Failed to load image: {}", path)` |
| `ar::critical()` | 最高 | 致命错误 | `ar::critical("System crash!")` |

---

## 实际开发建议

### 1. 合理选择日志级别

```cpp
// ✅ 好的实践
ar::info("Task started");           // 重要流程节点
ar::debug("Loop iteration: {}", i); // 调试细节
ar::error("Failed to connect");     // 错误情况

// ❌ 不好的实践
ar::info("i = {}", i);  // 应该用 debug
ar::debug("Critical error occurred");  // 应该用 error
```

### 2. 避免过度日志

```cpp
// ❌ 不好：每帧都输出日志（60 FPS = 每秒 60 次）
while (running) {
    ar::info("Frame: {}", frame_count++);  // 日志洪水！
}

// ✅ 好：定期输出或条件输出
if (frame_count % 60 == 0) {
    ar::info("FPS: {}", 60.0 / elapsed_time);
}
```

### 3. 格式化字符串优化

```cpp
// ❌ 不好：字符串拼接（性能差）
ar::info(std::string("x: ") + std::to_string(x) + ", y: " + std::to_string(y));

// ✅ 好：格式化占位符（零拷贝）
ar::info("x: {}, y: {}", x, y);
```

---

## 总结

`ar::info()` 函数体现了**现代 C++ 的最佳实践**：

| 特性 | 技术实现 | 优势 |
|------|---------|------|
| **可变参数** | `template <typename... Args>` | 支持任意数量参数 |
| **完美转发** | `Args&&...` | 避免不必要的拷贝 |
| **类型安全** | `std::format_string<Args...>` | 编译期类型检查 |
| **零开销抽象** | 模板内联 + 编译器优化 | 性能接近 `printf` |
| **统一接口** | `detail::callMiniLog` | 减少模板实例化 |

在 OpenAR 框架中，它是连接用户代码和底层 spdlog 库的**类型安全桥梁**，既保证了开发体验，又维持了高性能。