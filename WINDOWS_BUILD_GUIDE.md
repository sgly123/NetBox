# 🪟 NetBox Windows 构建指南

> **Windows平台下的完整构建和使用指南**

## 🎯 **快速开始**

### **使用NetBox CLI（推荐）**
```powershell
# 创建项目
.\netbox.ps1 init MyProject

# 进入项目目录
cd MyProject

# 构建项目（自动处理Windows特殊性）
.\netbox.ps1 build

# 运行程序
.\netbox.ps1 run
```

---

## 🔧 **手动构建方式**

### **方法1：CMake通用构建（推荐）**
```cmd
# 配置项目
mkdir build
cd build
cmake ..

# 构建项目（适用于所有生成器）
cmake --build . --config Release

# 并行构建（4个线程）
cmake --build . --config Release --parallel 4
```

### **方法2：Visual Studio**
```cmd
# 生成Visual Studio项目
cmake .. -G "Visual Studio 17 2022"

# 使用MSBuild构建
msbuild ALL_BUILD.vcxproj /p:Configuration=Release

# 或者打开.sln文件在Visual Studio中构建
start MyProject.sln
```

### **方法3：MinGW**
```cmd
# 使用MinGW生成器
cmake .. -G "MinGW Makefiles"

# 使用MinGW构建
mingw32-make

# 或者
make
```

### **方法4：Ninja（高性能）**
```cmd
# 安装Ninja: choco install ninja
cmake .. -G "Ninja"

# 构建
ninja
```

---

## 📁 **Windows文件结构**

### **构建后的目录结构**
```
MyProject/
├── build/
│   ├── Debug/              # Debug版本可执行文件
│   │   ├── MyProject.exe
│   │   └── MyProject_example.exe
│   ├── Release/            # Release版本可执行文件
│   │   ├── MyProject.exe
│   │   └── MyProject_example.exe
│   └── MyProject.sln       # Visual Studio解决方案文件
├── src/
├── include/
└── CMakeLists.txt
```

### **运行程序**
```cmd
# Debug版本
build\Debug\MyProject.exe

# Release版本
build\Release\MyProject.exe

# 使用NetBox CLI
.\netbox.ps1 run
```

---

## 🛠️ **环境准备**

### **必需工具**
1. **CMake** (3.16+)
   ```powershell
   # 使用Chocolatey安装
   choco install cmake
   
   # 或从官网下载: https://cmake.org/download/
   ```

2. **C++编译器**（选择其一）
   - **Visual Studio 2019/2022** (推荐)
   - **MinGW-w64**
   - **Clang**

3. **Python** (3.7+)
   ```powershell
   # 从官网下载: https://www.python.org/downloads/
   ```

### **可选工具**
- **Ninja** - 更快的构建系统
  ```powershell
  choco install ninja
  ```
- **Git** - 版本控制
  ```powershell
  choco install git
  ```

---

## 🎯 **构建配置选项**

### **构建类型**
```cmd
# Debug构建（包含调试信息）
cmake --build . --config Debug

# Release构建（优化性能）
cmake --build . --config Release

# RelWithDebInfo（发布版本+调试信息）
cmake --build . --config RelWithDebInfo

# MinSizeRel（最小体积）
cmake --build . --config MinSizeRel
```

### **生成器选择**
```cmd
# Visual Studio 2022
cmake .. -G "Visual Studio 17 2022"

# Visual Studio 2019
cmake .. -G "Visual Studio 16 2019"

# MinGW
cmake .. -G "MinGW Makefiles"

# Ninja（需要先安装）
cmake .. -G "Ninja"

# NMake（Visual Studio命令提示符中）
cmake .. -G "NMake Makefiles"
```

### **架构选择**
```cmd
# 64位（默认）
cmake .. -A x64

# 32位
cmake .. -A Win32

# ARM64
cmake .. -A ARM64
```

---

## 🚀 **性能优化**

### **并行构建**
```cmd
# 使用所有CPU核心
cmake --build . --config Release --parallel

# 指定线程数
cmake --build . --config Release --parallel 8
```

### **编译器优化**
```cmd
# 启用链接时优化
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON

# 指定优化级别
cmake .. -DCMAKE_CXX_FLAGS="/O2"
```

---

## 🐛 **常见问题解决**

### **问题1：cmake不是内部或外部命令**
```powershell
# 解决方案：安装CMake并添加到PATH
choco install cmake
# 或手动添加CMake到系统PATH
```

### **问题2：找不到编译器**
```powershell
# 解决方案：安装Visual Studio或MinGW
# Visual Studio: https://visualstudio.microsoft.com/
# MinGW: https://www.mingw-w64.org/
```

### **问题3：编码问题**
```cmd
# 在命令提示符中设置UTF-8编码
chcp 65001
```

### **问题4：权限问题**
```powershell
# 以管理员身份运行PowerShell
# 或修改执行策略
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### **问题5：找不到可执行文件**
```cmd
# 检查构建是否成功
dir build\Debug\*.exe
dir build\Release\*.exe

# 或使用NetBox CLI自动查找
.\netbox.ps1 run
```

---

## 🎨 **IDE集成**

### **Visual Studio**
```cmd
# 生成Visual Studio项目
cmake .. -G "Visual Studio 17 2022"

# 打开解决方案文件
start MyProject.sln
```

### **Visual Studio Code**
1. 安装C/C++扩展
2. 安装CMake Tools扩展
3. 打开项目文件夹
4. 使用Ctrl+Shift+P -> "CMake: Configure"

### **CLion**
1. 直接打开项目根目录
2. CLion会自动识别CMakeLists.txt
3. 选择构建配置并构建

---

## 📊 **性能对比**

| 构建系统 | 配置时间 | 构建速度 | 推荐场景 |
|----------|----------|----------|----------|
| Visual Studio | 中等 | 中等 | 开发调试 |
| MinGW | 快 | 中等 | 轻量级开发 |
| Ninja | 快 | 最快 | 大型项目 |
| NMake | 快 | 慢 | 简单项目 |

---

## 🎯 **最佳实践**

### **开发环境**
```cmd
# 推荐配置
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
```

### **生产环境**
```cmd
# 推荐配置
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### **CI/CD环境**
```cmd
# 推荐配置
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel --verbose
```

---

## 🚀 **一键脚本**

创建 `build.bat` 文件：
```batch
@echo off
echo 🔨 NetBox Windows 一键构建脚本
echo ================================

if not exist build mkdir build
cd build

echo ⚙️  配置项目...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto error

echo 🔧 构建项目...
cmake --build . --config Release --parallel
if errorlevel 1 goto error

echo ✅ 构建成功!
echo 🎯 可执行文件: build\Release\
goto end

:error
echo ❌ 构建失败!
exit /b 1

:end
pause
```

---

<div align="center">

**🪟 NetBox Windows构建指南**

**从配置到运行的完整解决方案** 🚀

**现在在Windows上也能享受丝滑的开发体验！**

</div>
