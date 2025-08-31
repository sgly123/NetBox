#!/bin/bash

# NetBox 单元测试运行脚本
# 运行所有单元测试并生成报告

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 测试结果统计
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

echo -e "${BLUE}======================================${NC}"
echo -e "${BLUE}    NetBox 单元测试执行报告${NC}"
echo -e "${BLUE}======================================${NC}"
echo ""

# 确保在build目录中
if [ ! -f "Makefile" ]; then
    echo -e "${RED}错误: 请在build目录中运行此脚本${NC}"
    exit 1
fi

# 测试套件列表
TEST_SUITES=(
    "test_base:基础组件测试"
    "test_util:工具类测试"
    "test_io:IO多路复用测试"
    "test_protocol:协议层测试"
    "test_server:服务器组件测试"
    "test_app:应用层测试"
    "test_integration:集成测试"
    "test_performance:性能测试"
)

# 运行每个测试套件
for suite_info in "${TEST_SUITES[@]}"; do
    IFS=':' read -r suite_name suite_desc <<< "$suite_info"
    
    echo -e "${YELLOW}运行 $suite_desc ($suite_name)...${NC}"
    
    # 运行测试并捕获输出
    if ./tests/bin/$suite_name > /tmp/${suite_name}_output.txt 2>&1; then
        # 测试通过
        passed=$(grep -o "\[  PASSED  \] [0-9]* tests" /tmp/${suite_name}_output.txt | grep -o "[0-9]*" || echo "0")
        echo -e "${GREEN}✓ $suite_desc: $passed 个测试通过${NC}"
        PASSED_TESTS=$((PASSED_TESTS + passed))
        TOTAL_TESTS=$((TOTAL_TESTS + passed))
    else
        # 测试失败
        passed=$(grep -o "\[  PASSED  \] [0-9]* tests" /tmp/${suite_name}_output.txt | grep -o "[0-9]*" || echo "0")
        failed=$(grep -o "\[  FAILED  \] [0-9]* tests" /tmp/${suite_name}_output.txt | grep -o "[0-9]*" || echo "0")
        total=$(grep -o "[0-9]* tests from [0-9]* test suites ran" /tmp/${suite_name}_output.txt | grep -o "^[0-9]*" || echo "0")
        
        echo -e "${RED}✗ $suite_desc: $passed 个测试通过, $failed 个测试失败${NC}"
        
        # 显示失败的测试
        if [ "$failed" -gt 0 ]; then
            echo -e "${RED}  失败的测试:${NC}"
            grep "\[  FAILED  \]" /tmp/${suite_name}_output.txt | grep -v "tests, listed below" | sed 's/^/    /'
        fi
        
        PASSED_TESTS=$((PASSED_TESTS + passed))
        FAILED_TESTS=$((FAILED_TESTS + failed))
        TOTAL_TESTS=$((TOTAL_TESTS + total))
    fi
    
    echo ""
done

# 生成总结报告
echo -e "${BLUE}======================================${NC}"
echo -e "${BLUE}           测试结果总结${NC}"
echo -e "${BLUE}======================================${NC}"
echo -e "总测试数: $TOTAL_TESTS"
echo -e "${GREEN}通过测试: $PASSED_TESTS${NC}"
echo -e "${RED}失败测试: $FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}🎉 所有测试都通过了！${NC}"
    SUCCESS_RATE=100
else
    SUCCESS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))
    echo -e "${YELLOW}成功率: $SUCCESS_RATE%${NC}"
fi

echo ""

# 生成详细报告文件
REPORT_FILE="test_report_$(date +%Y%m%d_%H%M%S).txt"
echo "NetBox 单元测试报告" > $REPORT_FILE
echo "生成时间: $(date)" >> $REPORT_FILE
echo "======================================" >> $REPORT_FILE
echo "" >> $REPORT_FILE

for suite_info in "${TEST_SUITES[@]}"; do
    IFS=':' read -r suite_name suite_desc <<< "$suite_info"
    echo "=== $suite_desc ===" >> $REPORT_FILE
    if [ -f "/tmp/${suite_name}_output.txt" ]; then
        cat /tmp/${suite_name}_output.txt >> $REPORT_FILE
    fi
    echo "" >> $REPORT_FILE
done

echo "总结:" >> $REPORT_FILE
echo "总测试数: $TOTAL_TESTS" >> $REPORT_FILE
echo "通过测试: $PASSED_TESTS" >> $REPORT_FILE
echo "失败测试: $FAILED_TESTS" >> $REPORT_FILE
echo "成功率: $SUCCESS_RATE%" >> $REPORT_FILE

echo -e "${BLUE}详细报告已保存到: $REPORT_FILE${NC}"

# 清理临时文件
rm -f /tmp/test_*_output.txt

# 返回适当的退出码
if [ $FAILED_TESTS -eq 0 ]; then
    exit 0
else
    exit 1
fi
