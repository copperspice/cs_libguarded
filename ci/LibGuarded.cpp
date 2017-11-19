#include "libguarded/guarded.hpp"

struct Foo
{
    int value;
};

int main()
{
    
    libguarded::guarded<Foo> m_foo;

    auto lock = m_foo.lock();
    return 0;
}

