a. phandle : // 节点中的phandle属性, 它的取值必须是唯一的(不要跟其他的phandle值一样)

pic@10000000 {
    phandle = <1>;
    interrupt-controller;
};

another-device-node {
    interrupt-parent = <1>;   // 使用phandle值为1来引用上述节点
};

b. label:

PIC: pic@10000000 {
    interrupt-controller;
};

another-device-node {
    interrupt-parent = <&PIC>;   // 使用label来引用上述节点, 
                                 // 使用lable时实际上也是使用phandle来引用, 
                                 // 在编译dts文件为dtb文件时, 编译器dtc会在dtb中插入phandle属性
};