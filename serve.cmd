@echo off
REM ============================================================
REM  Local Jekyll preview server
REM  Requires portable Ruby (RubyInstaller 3.3 + MSYS2 toolchain).
REM  Override RUBY_VER below if you installed a different version.
REM ============================================================
setlocal
set RUBY_VER=3.3.13
set RUBY_HOME=%USERPROFILE%\.workbuddy\binaries\ruby\%RUBY_VER%

if not exist "%RUBY_HOME%\bin\ruby.exe" (
  echo [ERROR] Portable Ruby not found: %RUBY_HOME%
  echo         Edit RUBY_VER at the top of this script.
  exit /b 1
)

set RI_DEVKIT=%RUBY_HOME%\msys64
set PATH=%RI_DEVKIT%\ucrt64\bin;%RI_DEVKIT%\usr\bin;%RUBY_HOME%\bin;%PATH%

cd /d "%~dp0docs"

if not exist "Gemfile.lock" (
  echo [1/2] Installing gems ^(first run only^)...
  call bundle install
  if errorlevel 1 exit /b 1
)

echo [2/2] Serving on http://127.0.0.1:4000  ^(Ctrl+C to stop^)
call bundle exec jekyll serve --host 127.0.0.1 --port 4000 --livereload --force_polling
