#include "m_pd.h"
#include <mysofa.h>
#include <stdlib.h>

static t_class *readsofa_class;

typedef struct _readsofa
{
    t_object x_obj;
    t_float x;
    t_float y;
    t_float z;
    t_symbol *path;
    int filter_length;
    int err;
    struct MYSOFA_EASY *hrtf;
    t_outlet *l_out;
    t_outlet *r_out;
    t_outlet *ld_out;
    t_outlet *rd_out;
} t_readsofa;

void readsofa_bang(t_readsofa *x)
{
    if(x->hrtf == NULL)
        x->hrtf = mysofa_open((x->path)->s_name, 48000, &x->filter_length, &x->err);
    error((x->path)->s_name);

    float leftIR[x->filter_length]; // [-1. till 1]
    float rightIR[x->filter_length];
    float leftDelay = 0;  // unit is sec.
    float rightDelay = 0; // unit is sec.

    if (x->hrtf != NULL)
    {
        mysofa_getfilter_float(x->hrtf, (float)x->x, (float)x->y, (float)x->z, leftIR, rightIR, &leftDelay, &rightDelay);
    }

    int l = x->filter_length;
    t_atom argv[l];
    for (size_t i = 0; i < l; i++)
    {
        SETFLOAT(argv + i, leftIR[i]);
    }
    outlet_list(x->l_out, gensym("list"), l, argv);

    for (size_t i = 0; i < l; i++)
    {
        SETFLOAT(argv + i, rightIR[i]);
    }
    outlet_list(x->r_out, gensym("list"), l, argv);
    outlet_float(x->ld_out, leftDelay);
    outlet_float(x->rd_out, rightDelay);
}

void *readsofa_new(t_symbol *s, t_int argc, t_atom *argv)
{
    t_readsofa *x = (t_readsofa *)pd_new(readsofa_class);

    x->path = atom_getsymbol(argv + 1);
    x->x = atom_getfloat(argv + 2);
    x->y = atom_getfloat(argv + 3);
    x->z = atom_getfloat(argv + 4);

    symbolinlet_new(&x->x_obj, &x->path);
    floatinlet_new(&x->x_obj, &x->x);
    floatinlet_new(&x->x_obj, &x->y);
    floatinlet_new(&x->x_obj, &x->z);

    x->l_out = outlet_new(&x->x_obj, &s_list);
    x->r_out = outlet_new(&x->x_obj, &s_list);
    x->ld_out = outlet_new(&x->x_obj, &s_float);
    x->rd_out = outlet_new(&x->x_obj, &s_float);
    return (void *)x;
}

void readsofa_setup(void)
{
    readsofa_class = class_new(gensym("readsofa"),
                                (t_newmethod)readsofa_new,
                                0, sizeof(t_readsofa),
                                CLASS_DEFAULT,
                                A_GIMME, 0);

    class_addbang(readsofa_class, readsofa_bang);
}