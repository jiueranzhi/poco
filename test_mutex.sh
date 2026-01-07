#!/bin/bash
echo "运行 10 次测试，观察没有 Mutex 时是否会出错..."
echo ""

for i in 1 2 3 4 5 6 7 8 9 10
do
    ./mutex_demo 2>&1 | grep -A2 "【测试1】" | tail -2
    echo "---"
done
