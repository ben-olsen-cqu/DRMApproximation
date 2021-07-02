#include "../Headers/Params.h"
#include "../Headers/os.h"
#include "../Headers/FileReader.h"
#include "../Headers/Coordinates.h"
#include "../Headers/Bit.h"

#include <iostream>
#include <exception>

void CommandLineArgs(ParamManager* pm, int argc, char* argv[])
{
    Parameter p1("Path", ">", ParamType::Text);
    Parameter p2("Var", "v", ParamType::Bool);
    
    pm->Store(p1);
    pm->Store(p2);
    
    pm->Set(argc, argv);
    if (pm->GetbyName("Path").RetrieveS().size() > 0)
    {
        pm->Output();
    }
}

int main(int argc, char* argv[])
{
    ParamManager pm;
    std::vector<std::string> files;
    std::vector<Coordinates> Coords;

    CommandLineArgs(&pm, argc, argv);

    if (pm.GetbyName("Path").RetrieveS().size() < 1)
        return -1;

    try
    {
        files = FileReader::GetFileList(pm.GetbyName("Path").RetrieveS());
    }
    catch(std::exception ex)
    {
        std::cout << ex.what() << "\n" << pm.GetbyName("Path").RetrieveS() << "\n";
        return -1;
    }

    std::cout << "Found total of " << std::to_string(files.size()) << " files to proccess." << std::endl << std::endl;

    Coords = FileReader::ReadFiles(files);

    std::cout << std::endl << "All files finished importing. Total coordinates loaded: " << Coords.size() << std::endl << std::endl;

    return 0;
}
