#ifndef ARRAY_OPT_H
#define ARRAY_OPT_H

#ifndef byte
#define byte unsigned char
#endif // byte

struct Arr
{
    byte *pBase;//存储的是数组第一个元素的地址
    int len;//数组所能容纳的最大元素的个数
    int cnt;//当前数组有效元素的个数
    size_t elm_size;
};

int init_arr(struct Arr *pArr, size_t elm_size);
bool append_arr(struct Arr *pArr,void* value);
bool insert_arr(struct Arr *pArr,int pos,void* value);
bool delete_arr(struct Arr *pArr,int pos,void * pDelElm);
bool is_empty(struct Arr *pArr);
bool is_full(struct Arr *pArr);
void sort_arr(struct Arr *pArr,__compar_fn_t fun);
int find(struct Arr *pArr, void* keyword, __compar_fn_t fun);

#endif // ARRAY_OPT_H