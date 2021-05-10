#include "common.h"

void CommonAddRef(Common* self)
{
    self->RefCount++;
}

unsigned int CommonRelease(Common* self)
{
    self->RefCount--;
    if (self->RefCount == 0)
    {
        if (self->lpVtbl)
        {
            free(self->lpVtbl);
        }
        free(self);
        return 0;
    }
    return self->RefCount;
}

Common* _Common()
{
    // Allocate object
    Common* This = calloc(1, sizeof(Common));
    HARDEXITONALLOCFAIL(This);

    // Populate members
    This->RefCount = 0;

    // Allocate virtual table
    CommonVtbl* Vtbl = calloc(1, sizeof(CommonVtbl));
    HARDEXITONALLOCFAIL(Vtbl);

    // Populate virtual table
    Vtbl->AddRef = CommonAddRef;
    Vtbl->Release = CommonRelease;

    // Assign virtual table to object
    This->lpVtbl = Vtbl;

    // Increase object reference count
    This->lpVtbl->AddRef(This);
    return This;
}