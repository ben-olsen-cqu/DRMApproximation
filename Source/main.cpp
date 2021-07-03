#include "../Headers/Params.h"
#include "../Headers/os.h"
#include "../Headers/FileReader.h"
#include "../Headers/Coordinates.h"
#include "../Headers/Bit.h"
#include "../Headers/Quadtree.h"

#include <iostream>
#include <exception>
#include <filesystem>

void CommandLineArgs(ParamManager* pm, int argc, char* argv[])
{
    Parameter p1("Path", ">", ParamType::Text);
    Parameter p2("Var", "v", ParamType::Bool);
    
    pm->Store(p1);
    pm->Store(p2);
    
    pm->Set(argc, argv);
    
    #if defined(MAC)
    std::filesystem::path relpath = pm->GetbyName("Path").RetrieveS();
    std::filesystem::path absolutePath = argv[0];
    absolutePath.remove_filename();
    absolutePath /= relpath;
    pm->SetPath(absolutePath.lexically_normal());
    #endif
    
    if (pm->GetbyName("Path").RetrieveS().size() > 0)
    {
        pm->Output();
    }
}

int main(int argc, char* argv[])
{
    ParamManager pm; //Initialise a parammanager to handle the command line args passed in at runtime
    std::vector<std::string> files; //list of strings for input file paths
    std::vector<Coordinates> Coords; //list of coordinates to store in the quadtree structure

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

    std::cout << "Size of a Bit Quadtree: " << sizeof(Quadtree<Bit>) << " Bytes\n";
    std::cout << "Size of a Coordinates Quadtree: " << sizeof(Quadtree<Coordinates>) << " Bytes\n";
    std::cout << "Size of a Bit Type: " << sizeof(Bit) << " Bytes\n";
    std::cout << "Size of a Coordinates Type: " << sizeof(Coordinates) << " Bytes\n";


    return 0;
}
