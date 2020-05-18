#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <errno.h>

#include "array_opt.h"
#define DEF_LEN (10)
int init_arr(struct Arr *pArr, size_t elm_size)
{
    pArr->pBase = (byte *)malloc(elm_size * DEF_LEN);
    if(pArr->pBase == NULL)
    {
        return -errno;
    }
    pArr->elm_size = elm_size;
    pArr->cnt = 0;
    pArr->len = DEF_LEN;
    return 0;
}
bool is_empty(struct Arr *pArr)
{
    return (pArr->cnt==0);
}
bool is_full(struct Arr *pArr)
{
    return ((pArr->cnt)==(pArr->len));
}
bool append_arr(struct Arr *pArr, void* value)
{
    if(is_full(pArr))
    {
            printf("%s:%d %s\n",__FILE__,__LINE__,__FUNCTION__);
        byte* tmp = (byte *)malloc(pArr->elm_size * pArr->len * 2);
            printf("%s:%d %s\n",__FILE__,__LINE__,__FUNCTION__);
        if(tmp == NULL)
        {
            return false;
        }
        memcpy(tmp, pArr->pBase, pArr->elm_size * pArr->len);
        pArr->len *= 2;
        free(pArr->pBase);
        pArr->pBase = tmp;
    }

    memcpy(&(pArr->pBase[pArr->elm_size*pArr->cnt]),value,pArr->elm_size);
    (pArr->cnt)++;
    return true;
}
bool insert_arr(struct Arr *pArr,int pos, void* value)
{
    if(pos<0 || pos >= pArr->cnt)
        return false;
    if(is_full(pArr))
    {
            printf("%s:%d %s\n",__FILE__,__LINE__,__FUNCTION__);
        byte* tmp = (byte *)malloc(pArr->elm_size * pArr->len * 2);
            printf("%s:%d %s\n",__FILE__,__LINE__,__FUNCTION__);
        if(tmp == NULL)
        {
            return false;
        }
        memcpy(tmp, pArr->pBase, pArr->elm_size * pArr->len);
        pArr->len *= 2;
        free(pArr->pBase);
        pArr->pBase = tmp;
    }
    memmove(
        &(pArr->pBase[pArr->elm_size*pos]),
        &(pArr->pBase[pArr->elm_size*(pos+1)]),
        pArr->elm_size*(pArr->cnt - pos - 1));

    memcpy(&(pArr->pBase[pArr->elm_size*pos]),value,pArr->elm_size);
    (pArr->cnt)++;
    return true;
}
bool delete_arr(struct Arr *pArr,int pos, void * pDelElm)
{
    if(pos<0 || pos>=pArr->cnt)
        return false;
    if( is_empty(pArr))
        return false;
    if(pDelElm!=NULL)
    {
        memcpy(
            pDelElm,
            &(pArr->pBase[pArr->elm_size*pos]),
            pArr->elm_size);
    }
    if(pos < pArr->cnt -1 )
    {
        memmove(
            &(pArr->pBase[pArr->elm_size*pos]),
            &(pArr->pBase[pArr->elm_size*(pos+1)]),
            pArr->elm_size*(pArr->cnt - pos - 1));
    }
    (pArr->cnt)--;
    return true;
}
void sort_arr(struct Arr *pArr,__compar_fn_t fun)
{
    qsort(pArr->pBase,pArr->cnt,pArr->elm_size,fun);
}
int find(struct Arr *pArr, void* keyword, __compar_fn_t fun)
{
    for(int i=0;i<pArr->cnt;i++)
    {
        if(fun(keyword,&(pArr->pBase[pArr->elm_size*i]))==0)
            return i;
    }
    return -1;
}