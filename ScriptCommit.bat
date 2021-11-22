@echo off
@chcp 65001
git add .
set /p CommitMsg="输入提交消息:"
git commit -m %CommitMsg%
echo.
git push origin main
git status
pause