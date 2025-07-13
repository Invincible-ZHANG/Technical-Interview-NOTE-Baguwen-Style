#!/bin/bash

# 自动拉取最新代码
git pull

# 添加所有更改
git add .

# 提交，附带自动时间戳
git commit -m "Auto update: $(date '+%Y-%m-%d %H:%M:%S')"

# 推送到远程仓库
git push
