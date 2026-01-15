# WiFi Brute Forcer - Educational Research Tool

## 项目概述

这是一个用于教育研究目的的WiFi密码测试工具，使用标准C++17开发，支持Windows 11 64位系统。该工具仅用于合法授权的网络安全测试和教育研究，使用者需确保遵守当地法律法规并获得适当授权。

## 功能特点

- **支持WPA/WPA2加密类型**：实现对常见WiFi加密类型的密码测试
- **字典攻击**：支持从文本文件导入密码字典
- **命令行界面**：提供丰富的参数配置选项
- **多线程支持**：实现合理的线程管理以提高测试效率
- **进度显示**：实时显示测试进度
- **结果记录**：自动保存成功的密码到文件
- **网络列表**：支持列出可用的WiFi网络

## 技术规格

- **编程语言**：C++17
- **目标平台**：Windows 11 (64位)
- **编译工具**：MinGW-w64 / Clang (禁止使用Visual Studio)
- **依赖库**：Windows WLAN API
- **构建系统**：CMake

## 目录结构

```
WiFiBruteForcer/
├── include/          # 头文件目录
│   ├── wifi.h        # WiFi连接和爆破功能头文件
│   └── args.h        # 命令行参数解析头文件
├── src/              # 源文件目录
│   ├── main.cpp      # 主程序入口
│   ├── wifi.cpp      # WiFi功能实现
│   └── args.cpp      # 参数解析实现
├── docs/             # 文档目录
├── build/            # 构建目录
├── CMakeLists.txt    # CMake构建脚本
├── resources.rc.in   # Windows资源文件模板
└── README.md         # 项目说明文档
```

## 编译说明

### 前置条件

- **MinGW-w64**：下载并安装MinGW-w64编译器套件
  - 推荐使用x86_64-w64-mingw32
  - 确保将MinGW-w64的bin目录添加到系统PATH

- **CMake**：下载并安装CMake 3.10或更高版本
  - 确保将CMake的bin目录添加到系统PATH

### 编译步骤

1. **打开命令行终端**（如Windows Terminal、PowerShell）

2. **导航到项目目录**
   ```bash
   cd e:/project/WIFIpassword
   ```

3. **创建并进入构建目录**
   ```bash
   mkdir -p build
   cd build
   ```

4. **运行CMake配置**
   ```bash
   cmake -G "MinGW Makefiles" ..
   ```
   或使用Clang：
   ```bash
   cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=clang++ ..
   ```

5. **编译项目**
   ```bash
   mingw32-make
   ```

6. **获取可执行文件**
   编译完成后，可执行文件将生成在 `build/bin/` 目录下：`wifi_brute.exe`

## 使用指南

### 基本语法

```bash
wifi_brute [选项]
```

### 命令行选项

| 选项 | 长选项 | 描述 |
|------|--------|------|
| `-h` | `--help` | 显示帮助信息 |
| `-v` | `--version` | 显示版本信息 |
| `-l` | `--list` | 列出可用的WiFi网络 |
| `-s` | `--ssid <SSID>` | 指定目标WiFi的SSID |
| `-d` | `--dictionary <路径>` | 指定密码字典文件路径 |
| `-t` | `--timeout <毫秒>` | 设置连接超时时间（默认：5000） |
| `-c` | `--threads <数量>` | 设置线程数量（默认：4） |

### 使用示例

1. **列出可用WiFi网络**
   ```bash
   wifi_brute --list
   ```

2. **基本密码测试**
   ```bash
   wifi_brute --ssid MyWiFi --dictionary passwords.txt
   ```

3. **自定义超时和线程数**
   ```bash
   wifi_brute --ssid MyWiFi --dictionary passwords.txt --timeout 3000 --threads 8
   ```

### 字典文件格式

密码字典文件应为纯文本文件，每行包含一个密码：

```
password123
admin123
welcome
letmein
12345678
```

建议使用常见的密码字典，如：
- rockyou.txt
- 弱密码字典
- 自定义生成的密码列表

## 功能说明

### WiFi连接功能

该工具使用Windows WLAN API实现WiFi连接功能：
- 自动检测WiFi接口
- 支持创建和管理WiFi配置文件
- 实现连接状态检测
- 自动断开连接

### 密码测试机制

1. **字典加载**：从指定文件加载密码列表
2. **线程分配**：将密码列表分配给多个线程
3. **并行测试**：多个线程同时测试不同密码
4. **结果验证**：检测连接状态以验证密码正确性
5. **进度显示**：实时显示测试进度百分比
6. **结果保存**：将成功的密码保存到 `wifi_brute_result.txt` 文件

### 线程管理

- 默认使用4个线程
- 可通过命令行参数调整线程数量
- 每个线程独立测试一部分密码
- 找到正确密码后立即停止所有线程

### 错误处理

- 详细的错误信息输出
- 连接失败时的重试机制
- 字典文件不存在的错误处理
- WiFi接口初始化失败的错误处理

## 注意事项

1. **合法使用**：该工具仅用于合法授权的网络安全测试和教育研究
2. **授权要求**：使用前必须获得网络所有者的明确授权
3. **法律责任**：使用者需自行承担因非法使用该工具而产生的法律责任
4. **性能影响**：测试过程可能会影响WiFi网络性能
5. **密码长度**：WPA/WPA2密码长度必须在8-63个字符之间

## 安全声明

本程序不包含任何未授权访问功能，仅用于授权环境下的安全测试。开发者不承担因使用本工具而产生的任何直接或间接损失。

## 版本历史

- **v1.0.0** (2026-01-15)
  - 安全漏洞修复版本
  - 修复XML注入漏洞
  - 增强敏感信息保护
  - 改进线程安全性
  - 完善输入验证
  - 修复资源泄漏问题

## 许可证

本项目采用GPL-3.0许可证，详情请参阅LICENSE文件。

## 联系方式

仅供教育研究使用，如有问题或建议，请通过合法渠道联系开发者。
