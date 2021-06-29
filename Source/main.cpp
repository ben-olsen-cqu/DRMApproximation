#include "../Headers/Params.h"
#include "../Headers/os.h"

#include <iostream>

//#if defined(MAC) || defined(LINUX)
//#endif 
//#if defined(WINDOWS)
//#endif

int main(int argc, char* argv[])
{
    //-v 3 --flowdirection "/Input/Test Data1/" -rt
    
    Parameter p1("reuse","r",ParamType::Bool);
    Parameter p2("flowdirection","f",ParamType::Bool);
    Parameter p3("usestoredtrees","v",ParamType::Bool);
    Parameter p4("reuse","r",ParamType::Bool);
    
    std::vector<std::string> args;
    for(int i = 1; i < argc;i++)
    {
        std::cout << argv[i] << "\n";
        std::string s = argv[i];
        args.push_back(s);
    }
    
    while(!args.empty())
    {
        for(int i = 0; i < args.size();i++)
        {
            if(args[i][0]== '-')
            {
                if(args[i][1]== '-')
                {
                    
                }
                else
                {
                    
                }
            }
            else
            {
                
            }
        }
    }
    
    return 0;
}
