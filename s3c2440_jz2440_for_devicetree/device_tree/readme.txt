a. phandle : // �ڵ��е�phandle����, ����ȡֵ������Ψһ��(��Ҫ��������phandleֵһ��)

pic@10000000 {
    phandle = <1>;
    interrupt-controller;
};

another-device-node {
    interrupt-parent = <1>;   // ʹ��phandleֵΪ1�����������ڵ�
};

b. label:

PIC: pic@10000000 {
    interrupt-controller;
};

another-device-node {
    interrupt-parent = <&PIC>;   // ʹ��label�����������ڵ�, 
                                 // ʹ��lableʱʵ����Ҳ��ʹ��phandle������, 
                                 // �ڱ���dts�ļ�Ϊdtb�ļ�ʱ, ������dtc����dtb�в���phandle����
};