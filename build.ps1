#!/usr/bin/env pwsh
# build.ps1 - Скрипт для быстрой сборки проекта ADUserManager с правильными настройками
# Автор: GitHub Copilot
# Дата: 12.06.2025

# Убеждаемся, что PowerShell использует правильную кодировку для кириллицы
chcp 65001 | Out-Null  # Установка кодовой страницы UTF-8
$OutputEncoding = [System.Text.Encoding]::UTF8
[System.Console]::OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::InputEncoding = [System.Text.Encoding]::UTF8

# Строгий режим для обнаружения ошибок
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Определение цветовых кодов для вывода
$successColor = "Green"
$errorColor = "Red"
$warningColor = "Yellow"
$infoColor = "Cyan"

# Функция для вывода сообщений с цветным форматированием
function Write-ColorMessage {
    param (
        [Parameter(Mandatory=$true)]
        [string]$Message,
        [Parameter(Mandatory=$false)]
        [string]$Color = "White"
    )
    
    Write-Host $Message -ForegroundColor $Color
}

# Функция для проверки наличия программы в системе
function Test-CommandExists {
    param (
        [Parameter(Mandatory=$true)]
        [string]$Command
    )
    
    $oldPreference = $ErrorActionPreference
    $ErrorActionPreference = "Stop"
    
    try {
        $null = Get-Command $Command -ErrorAction Stop
        return $true
    }
    catch {
        return $false
    }
    finally {
        $ErrorActionPreference = $oldPreference
    }
}

# Проверка необходимых зависимостей
Write-ColorMessage "Проверка необходимых зависимостей..." $infoColor

$dependencies = @{
    "cmake" = "CMake"
    "mingw32-make" = "MinGW"
}

$missingDeps = @()
foreach ($dep in $dependencies.Keys) {
    if (-not (Test-CommandExists $dep)) {
        $missingDeps += $dependencies[$dep]
    }
}

if ($missingDeps.Count -gt 0) {
    Write-ColorMessage "❌ Отсутствуют необходимые зависимости: $($missingDeps -join ', ')" $errorColor
    Write-ColorMessage "Пожалуйста, установите недостающие компоненты и повторите попытку." $errorColor
    exit 1
}

# Определение путей
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $scriptDir "build"

# Определение переменных окружения для Qt
$qtDir = "C:\Qt\6.9.1\mingw_64"
$mingwDir = "C:\Qt\Tools\mingw1310_64"

# Проверка наличия Qt
if (-not (Test-Path $qtDir)) {
    Write-ColorMessage "❌ Qt не найден по пути: $qtDir" $errorColor
    Write-ColorMessage "Пожалуйста, проверьте путь к Qt и измените его в скрипте при необходимости." $errorColor
    exit 1
}

# Проверка наличия MinGW
if (-not (Test-Path $mingwDir)) {
    Write-ColorMessage "❌ MinGW не найден по пути: $mingwDir" $errorColor
    Write-ColorMessage "Пожалуйста, проверьте путь к MinGW и измените его в скрипте при необходимости." $errorColor
    exit 1
}

# Установка переменных окружения
Write-ColorMessage "📌 Настройка переменных окружения..." $infoColor
$env:PATH = "$qtDir\bin;$mingwDir\bin;$env:SystemRoot\System32;$env:PATH"
Write-ColorMessage "PATH = $env:PATH" $infoColor

# Проверка наличия uic.exe
if (Test-CommandExists "uic") {
    Write-ColorMessage "✅ Qt UIC найден: $(Get-Command uic | Select-Object -ExpandProperty Source)" $successColor
} else {
    Write-ColorMessage "❌ Qt UIC не найден в PATH" $errorColor
    exit 1
}

# Очистка предыдущей сборки
Write-ColorMessage "🧹 Очистка предыдущей сборки..." $infoColor

if (Test-Path $buildDir) {
    try {
        # Удаляем CMake-кэш и другие файлы, но сохраняем директорию
        Get-ChildItem -Path $buildDir -Recurse | Remove-Item -Force -Recurse -ErrorAction Stop
        Write-ColorMessage "✅ Кэш предыдущей сборки успешно удален" $successColor
    }
    catch {
        Write-ColorMessage "⚠️ Не удалось полностью очистить директорию сборки: $_" $warningColor
        Write-ColorMessage "Создается новая директория сборки..." $infoColor
        
        # В случае ошибки при очистке, пробуем удалить и создать директорию заново
        Remove-Item -Path $buildDir -Force -Recurse -ErrorAction SilentlyContinue
        New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    }
}
else {
    # Создаем директорию сборки, если она не существует
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    Write-ColorMessage "✅ Создана директория сборки" $successColor
}

# Переход в директорию сборки
Write-ColorMessage "📂 Переход в директорию сборки: $buildDir" $infoColor
Set-Location -Path $buildDir

# Конфигурация проекта с CMake
Write-ColorMessage "🔨 Настройка проекта с CMake..." $infoColor

try {
    # Запускаем CMake для конфигурации проекта
    cmake -G "MinGW Makefiles" `
        -DCMAKE_PREFIX_PATH="$qtDir" `
        -DCMAKE_C_COMPILER="$mingwDir\bin\gcc.exe" `
        -DCMAKE_CXX_COMPILER="$mingwDir\bin\g++.exe" `
        -DCMAKE_MAKE_PROGRAM="$mingwDir\bin\mingw32-make.exe" `
        ..
    
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "❌ Ошибка при конфигурации проекта с CMake. Код выхода: $LASTEXITCODE" $errorColor
        exit 1
    }
    
    Write-ColorMessage "✅ Конфигурация CMake успешно завершена" $successColor
}
catch {
    Write-ColorMessage "❌ Произошла ошибка при конфигурации проекта: $_" $errorColor
    exit 1
}

# Сборка проекта
Write-ColorMessage "🔨 Сборка проекта..." $infoColor

try {
    # Определение количества процессоров для параллельной сборки
    $cpuCount = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
    $jobCount = [math]::Max(1, $cpuCount - 1) # Оставляем один процессор свободным
    
    # Запуск сборки
    mingw32-make -j $jobCount
    
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "❌ Ошибка при сборке проекта. Код выхода: $LASTEXITCODE" $errorColor
        exit 1
    }
    
    Write-ColorMessage "✅ Сборка проекта успешно завершена" $successColor
}
catch {
    Write-ColorMessage "❌ Произошла ошибка при сборке проекта: $_" $errorColor
    exit 1
}

# Вывод информации о результате сборки
$exePath = Join-Path $buildDir "ADUserManager.exe"
if (Test-Path $exePath) {
    Write-ColorMessage "🎉 Сборка успешно завершена!" $successColor
    Write-ColorMessage "📝 Исполняемый файл находится по пути: $exePath" $successColor
    
    # Предложение запустить программу
    $runApp = Read-Host "Запустить программу? (д/н)"
    if ($runApp -eq "д" -or $runApp -eq "y" -or $runApp -eq "yes" -or $runApp -eq "да") {
        try {
            Write-ColorMessage "🚀 Запуск программы..." $infoColor
            & $exePath
        }
        catch {
            Write-ColorMessage "❌ Ошибка при запуске программы: $_" $errorColor
        }
    }
}
else {
    Write-ColorMessage "⚠️ Сборка завершена, но исполняемый файл не найден" $warningColor
}

# Возвращаемся в исходную директорию
Set-Location -Path $scriptDir

# Развертывание зависимостей Qt
if (Test-Path $exePath) {
    Write-ColorMessage "📦 Проверка наличия зависимостей Qt..." $infoColor
    
    # Проверка наличия DLL-файлов в директории сборки и их копирование при необходимости
    $qtDlls = @(
        "Qt6Core.dll",
        "Qt6Gui.dll", 
        "Qt6Widgets.dll", 
        "Qt6Network.dll"
    )
    
    $needDeploy = $false
    foreach ($dll in $qtDlls) {
        $destPath = Join-Path $buildDir $dll
        if (-not (Test-Path $destPath)) {
            $needDeploy = $true
            break
        }
    }
    
    if ($needDeploy) {
        Write-ColorMessage "📦 Копирование необходимых DLL-файлов Qt..." $infoColor
        try {
            # Ручное копирование основных DLL-файлов Qt
            foreach ($dll in $qtDlls) {
                $sourcePath = Join-Path $qtDir "bin" $dll
                $destPath = Join-Path $buildDir $dll
                
                if (Test-Path $sourcePath) {
                    Copy-Item -Path $sourcePath -Destination $destPath -Force
                    Write-ColorMessage "   Скопирован файл: $dll" $infoColor
                } else {
                    Write-ColorMessage "⚠️ Файл не найден: $sourcePath" $warningColor
                }
            }
            
            # Проверка и копирование необходимых папок с плагинами
            $pluginFolders = @(
                "platforms",  # для qwindows.dll
                "styles",     # для стилей
                "imageformats" # для работы с изображениями
            )
            
            foreach ($folder in $pluginFolders) {
                $sourceFolder = Join-Path $qtDir "plugins" $folder
                $destFolder = Join-Path $buildDir $folder
                
                # Проверяем наличие папки назначения
                if (-not (Test-Path $destFolder -PathType Container)) {
                    # Создаем папку, если она не существует
                    New-Item -Path $destFolder -ItemType Directory -Force | Out-Null
                    
                    # Копируем содержимое папки
                    if (Test-Path $sourceFolder) {
                        Copy-Item -Path "$sourceFolder\*" -Destination $destFolder -Recurse -Force
                        Write-ColorMessage "   Скопирована папка: $folder" $infoColor
                    } else {
                        Write-ColorMessage "⚠️ Папка плагинов не найдена: $sourceFolder" $warningColor
                    }
                } else {
                    Write-ColorMessage "   Папка плагинов уже существует: $folder" $infoColor
                }
            }
            
            Write-ColorMessage "✅ Зависимости Qt успешно развернуты" $successColor
        }
        catch {
            Write-ColorMessage "❌ Ошибка при копировании зависимостей Qt: $_" $errorColor
        }
    } else {
        Write-ColorMessage "✅ Все зависимости Qt уже присутствуют в директории сборки" $successColor
    }
}

# Конец скрипта
exit 0
