
#include "test.h"
#include "host_io.h"

Test * Test::root = 0;


Test::Test(const char * name, Test * parent): name(name), children(0), next(0)
{
    Test * c;
    if(parent)
    {
        c = parent->children;
        if(!c) {parent->children = this; return;}
    }
    else
    {
        c = root;
        if(!c) {root = this; return;}
    }
    while(Test * n = c->next) c = n;
    c->next = this;
};


void Test::run()
{
    out.log("Test started.");
    Test * stack[8], * * sp = stack;
    int count = 0, failed = 0;
    Test * x = root;
    do
    {       
        for( ; x; x = x->next)
        {
            while(Test * c = x->children)
            {
                out.log(" ");
                out.log(x->name);
                out.log("(");
                *(sp++) = x;
                x = c;
            }
            
            out.log(x->name);
            out.log(" ");
            const char * e = x->exec();
            count++;
            if(e)
            {
                out.log(e);
                failed++;
            }
        }
        if(sp > stack)
        {
            x = (*--sp)->next;
            out.log(")");
            continue;
        } else break;
    } while(true);
    out.log(" Test completed.");
}

