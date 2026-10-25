@echo off
REM ============================================================
REM  One-shot build into docs\_site  (no server, no watching)
REM  Useful to verify the site compiles before pushing.
REM ============================================================
setlocal
set RUBY_VER=3.3.13
set RUBY_HOME=%USERPROFILE%\.workbuddy\binaries\ruby\%RUBY_VER%

if not exist "%RUBY_HOME%\bin\ruby.exe" (
  echo [ERROR] Portable Ruby not found: %RUBY_HOME%
  exit /b 1
)

set RI_DEVKIT=%RUBY_HOME%\msys64
set PATH=%RI_DEVKIT%\ucrt64\bin;%RI_DEVKIT%\usr\bin;%RUBY_HOME%\bin;%PATH%

cd /d "%~dp0docs"

if not exist "Gemfile.lock" (
  echo [1/2] Installing gems...
  call bundle install
  if errorlevel 1 exit /b 1
)

echo [2/2] Building...
call bundle exec jekyll build
if errorlevel 1 (
  echo.
  echo [FAILED] Build failed - see the error above.
  exit /b 1
)

echo.
echo [OK] Output: docs\_site
