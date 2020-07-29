参考[json-tutorial](https://github.com/miloyip/json-tutorial)项目写的一个json解析器

## 特点：
- 符合[标准](https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf)的JSON解析器
- 递归下降解析器
- 使用标准C (ANSI)
- 仅支持 UTF-8 JSON文本
- 仅支持以 double 存储 JSON number 类型

## 运行环境：
Linux

## 编译：
cmake . && make

## 运行测试：
./leptjson_test

## 其他：
`git commit --amend --no-edit   # 追加到上次commit。不会进入编辑器，直接进行提交,`

值得注意的点：
- 测试驱动开发。这是第一次进行实践