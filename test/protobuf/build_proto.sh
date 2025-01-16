#!/bin/bash

# 定义 protoc 路径
PROTOC="../../3rdparty/protobuf_3.21.11/bin/protoc"
PROTO_FILE="test_coro_rpc.proto"

# 检查 protoc 是否存在
if [ ! -f "$PROTOC" ]; then
    echo "Error: protoc compiler not found"
    exit 1
fi

# Generate C++ code
echo "Generating C++ pb code..."
$PROTOC --cpp_out . $PROTO_FILE || { echo "Failed to generate C++ code"; exit 1; }

# Generate Go code
echo "Generating Go pb code..."
$PROTOC --go_out . $PROTO_FILE || { echo "Failed to generate Go code"; exit 1; }

