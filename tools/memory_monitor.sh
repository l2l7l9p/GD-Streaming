#!/bin/bash

# 检查是否提供了要运行的命令
if [ $# -eq 0 ]; then
    echo "usage: $0 <COMMAND>"
    exit 1
fi

# 获取要运行的命令
TARGET_COMMAND="$1"

for i in $(seq 1 5)
do
	# 日志文件
	LOG_FILE="memory_$2_$i.log"

	# 清空或创建日志文件
	> "$LOG_FILE"

	# 启动目标进程
	echo "running: $TARGET_COMMAND"
	$TARGET_COMMAND &
	TARGET_PID=$!  # 获取目标进程的 PID

	# 检查是否成功启动进程
	if ! ps -p "$TARGET_PID" > /dev/null; then
		echo "cannot run the command: $TARGET_COMMAND"
		exit 1
	fi

	# 监控内存使用情况
	while ps -p "$TARGET_PID" > /dev/null; do
		# 获取内存使用情况（RSS 驻留集大小，单位 KB）
		MEMORY_USAGE=$(ps -o vsz= -p "$TARGET_PID")

		# 将结果写入日志文件
		echo "$MEMORY_USAGE" >> "$LOG_FILE"

		# 等待 2 秒
		sleep 2
	done

	echo "program ended."
done