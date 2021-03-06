
/*
r9寄存器又叫sb寄存器（静态基址寄存器），是ext内全局变量的基地址

BUG原因是ext中mr_helper()会修改r9寄存器的值，而调用mythroad中的函数时并没有恢复r9的值
目前仅对mr_c_function_load(0);初始化方式进行修复，传其它参数调用mr_c_function_load()的结果尚未研究

别名：
ext中的 mr_helper() 在mythroad中叫 mr_c_function()
ext的第一个函数是ext文件起始位置+8叫 mr_c_function_load() ,在mythroad中叫 mr_load_c_function()


mr_c_function_load()会回调mythroad中的_mr_c_function_new()把mr_helper()函数地址传回mythroad
此时_mr_c_function_new()会设置mr_c_function_P

_mr_c_function_new() 返回后再由ext设置mr_c_function_P的内容:
mr_c_function_P.ER_RW_Length(uint32类型偏移量4) = mr_helper_get_rw_len();
mr_c_function_P.start_of_ER_RW(void*类型偏移量0) =  mrc_malloc(mr_c_function_P.ER_RW_Length); //注意mrc_malloc()是ext内的

因为ext里所有的事件函数都是通过 mythroad调用 mr_helper() 分发
而 mr_helper() 进去后就会将r9寄存器的值备份到r10并设置为 mr_c_function_P.start_of_ER_RW 的值， 直到返回才会 mov r10,r9 恢复原来的r9值
之后在ext空间内调用mythroad层的函数时因为r9寄存的值没有恢复，导致mythroad层全局变量的引用全部跑飞


ext内的mrc_malloc和mrc_free经过如下包装
void *mrc_malloc(uint32 len) {
    uint32 *p = _mr_c_function_table.mr_malloc(len + sizeof(uint32));
    if (p) {
        *p = len;
        return (void *)(p + 1);
    }
    return p;
}

void mrc_free(void *p) {
    uint32 *t = (uint32 *)p - 1;
    _mr_c_function_table.mr_free(t, *t + sizeof(uint32));
}

解决办法：
因为r9始终为mr_c_function_P.start_of_ER_RW，所以在mythroad空间也能够使用，并且貌似是唯一可以使用的值
所以需要借助r9来恢复r9和r10

要求在进入 mr_helper() 前通过mr_c_function_P.start_of_ER_RW备份 r9和r10
ext调用mythroad时在mythroad空间通过r9 恢复 r9 r10

因为ext中始终都是以mr_c_function_P.start_of_ER_RW为基址向上访问，因此我们重新分配它，向下访问将能达到我们的目的
在mr_c_function_load()之后重新对此内存进行分配

20201021上面的方案证实在非插件化的mrp中有效，但是插件化mrp因为又套了一层ext，所以r9r10又失效了，mmp
套娃的ext我们不知道它在何时设置mr_c_function_P.start_of_ER_RW，也没办法在它设置之后实施上面的方案，如果它又套一层呢？
因为mr_c_function_P.start_of_ER_RW也是用malloc获取的内存，因此在所有malloc上都加上我们的数据，效率可能会低，没办法了

*/

#include "./include/fixR9.h"

#include "./include/mr.h"

#ifndef __GNUC__
extern int32 mr_c_function_load(int32 code);
#define C_FUNCTION_P() (*(((void **)mr_c_function_load) - 1))

static void *r9Ext;
static void *r10Ext;
#endif

static void *lr;
static void *r10Mythroad;

void fixR9_saveMythroad() {
    r10Mythroad = getR10();
}

void fixR9_begin(void *oldR9v, void *oldR10v, void *oldLR) {
#ifndef __GNUC__
    mr_c_function_st *p = C_FUNCTION_P();
    setR9(p->start_of_ER_RW);
    // 设置r9后回到mythroad，此时才可以访问全局变量
    setR10(r10Mythroad);
    r9Ext = oldR9v;
    r10Ext = oldR10v;
#endif
    lr = oldLR;
}

void *fixR9_end() {
    register void *ret = lr;  // 必需用寄存器先存起来
#ifndef __GNUC__
    setR9R10(r9Ext, r10Ext);
#endif
    return ret;
}
