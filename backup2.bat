@echo off
setlocal

:: ==========================================
:: 設定區
:: ==========================================
set "FILE_PREFIX=ProjectBackup"

:: ==========================================
:: 執行區 (呼叫 PowerShell 執行壓縮)
:: ==========================================
echo ==========================================
echo 正在啟動備份程序 (使用 PowerShell)...
echo 正在排除 debug, release, build, .git 等目錄...
echo ==========================================

:: 這一行指令會做三件事：
:: 1. 取得時間
:: 2. 設定檔名
:: 3. 找出所有檔案(排除垃圾檔)並壓縮
powershell -NoProfile -Command "& { $d = Get-Date -Format 'yyyyMMdd_HHmm'; $zipName = '%FILE_PREFIX%_' + $d + '.zip'; Write-Host '目標檔案: ' $zipName; Get-ChildItem -Path . -Exclude 'debug','release','.git','.vs','*.ncb','*.suo','*.user','*.sdf','build*','*.rar',$zipName | Compress-Archive -DestinationPath $zipName -Update; Write-Host '備份完成！' }"

echo.
if %ERRORLEVEL% == 0 (
    echo ==========================================
    echo 成功！請檢查資料夾下的 .zip 檔案
    echo ==========================================
) else (
    echo ==========================================
    echo 失敗。請確認您的 PowerShell 功能正常。
    echo ==========================================
)

pause