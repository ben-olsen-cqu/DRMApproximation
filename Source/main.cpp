#include "../Headers/Params.h"
#include "../Headers/os.h"

#include <iostream>
#include <exception>

//#if defined(MAC) || defined(LINUX)
//#endif 
//#if defined(WINDOWS)
//#endif

int main(int argc, char* argv[])
{
    //-v 3 --flowdirection "/Input/Test Data1/" -rt

    Parameter p1("reuse", "r", ParamType::Bool);
    Parameter p2("flowdirection", "f", ParamType::Bool);
    Parameter p3("usestoredtrees", "t", ParamType::Bool);
    Parameter p4("variable", "v", ParamType::Integer);
    Parameter p5("Path", ">", ParamType::Text);

    ParamManager pm;

    pm.Store(p1);
    pm.Store(p2);
    pm.Store(p3);
    pm.Store(p4);
    pm.Store(p5);

    pm.Set(argc, argv);
    pm.Output();

    return 0;
}
