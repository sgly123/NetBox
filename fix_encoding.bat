@echo off
chcp 65001 >nul
echo 🔧 NetBox Windows 编码修复工具
echo ================================

if not exist src (
    echo ❌ 未找到src目录，请在项目根目录中运行
    pause
    exit /b 1
)

echo 📝 修复源文件编码...

REM 使用PowerShell修复编码
powershell -Command "& {
    Write-Host '🔍 查找需要修复的文件...'
    
    $files = @()
    $files += Get-ChildItem -Path 'src' -Filter '*.cpp' -Recurse
    $files += Get-ChildItem -Path 'src' -Filter '*.c' -Recurse
    $files += Get-ChildItem -Path 'include' -Filter '*.h' -Recurse
    $files += Get-ChildItem -Path 'include' -Filter '*.hpp' -Recurse
    
    foreach ($file in $files) {
        Write-Host \"📄 修复: $($file.FullName)\"
        
        # 读取文件内容
        $content = Get-Content -Path $file.FullName -Raw -Encoding UTF8
        
        # 以UTF-8 BOM格式重新保存
        $utf8Bom = New-Object System.Text.UTF8Encoding $true
        [System.IO.File]::WriteAllText($file.FullName, $content, $utf8Bom)
    }
    
    Write-Host '✅ 编码修复完成!'
}"

echo.
echo 🎯 现在可以重新构建项目了:
echo    1. 清理构建: rmdir /s /q build
echo    2. 重新构建: netbox build
echo.
pause
