
class Test
{
private:
    const char * name;
    static Test * root;
    Test * children, * next;
protected:
    virtual const char * exec() { return 0;};
public:
    Test(const char * name, Test * parent = 0);
    static void run();
};


#define beginTest(name) class Test##name: public Test \
{ \
    virtual const char * exec()
#define endTest(name, parent) public: \
    Test##name(): Test(#name, &parent){}; \
} test##name
