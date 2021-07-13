#include "../Headers/Params.h"
#include "../Headers/os.h"
#include "../Headers/FileReader.h"
#include "../Headers/Coordinates.h"
#include "../Headers/Bit.h"
#include "../Headers/QuadtreeManager.h"

#include <iostream>
#include <exception>
#include <filesystem>

void PrintHelp()
{
    std::cout << "Use the Readme" << std::endl; //Update Later %%%%%
}

void CommandLineArgs(ParamManager* pm, int argc, char* argv[])
{
    /* DEFINE THE PARAMETERS THAT WILL BE USED IN THE PROGRAM */
    Parameter p1("Path", ">", ParamType::Text);
    Parameter p2("help", "h", ParamType::Bool);
    Parameter p3("treereuse", "t", ParamType::Bool);
    Parameter p4("ascfiles", "a", ParamType::Bool);
    Parameter p5("memorylimit", "m", ParamType::Integer);
    Parameter p6("spacing", "s", ParamType::Real);
    
    /* STORE IN MANAGER */
    pm->Store(p1);
    pm->Store(p2);
    pm->Store(p3);
    pm->Store(p4);
    pm->Store(p5);
    pm->Store(p6);
    
    /* SET THE PARAMETERS IF PASSED IN */
    pm->Set(argc, argv);
    
    #if defined(MAC)
    std::filesystem::path relpath = pm->GetbyName("Path").RetrieveS();
    std::filesystem::path absolutePath = argv[0];
    absolutePath.remove_filename();
    absolutePath /= relpath;
    pm->SetPath(absolutePath.lexically_normal());
    #endif
    
    /* IF HELP IS DEFINED, PRINT HELP */
    if (pm->GetbyName("help").RetrieveB())
    {
        PrintHelp();
    }

    /* CHECK IF PATH IS DEFINED, AND PLOT ALL PARAMETER VALUES IF IT IS*/
    if (pm->GetbyName("Path").RetrieveS().size() > 0)
    {
        pm->Output();
    }
}

int main(int argc, char* argv[])
{
    /* LOCAL VARIABLES*/

    ParamManager ParameterManager; //Initialise a param-manager to handle the command line args passed in at runtime
    std::vector<std::string> files; //list of strings for input file paths

    int maxMem = 1000; //Memory limit for trees, 2 trees max will be active at a time so half for each tree 
    float spacing = 1.0f; //Spacing between the points in the x and y directions, only square grids are supported

    /* COMMAND LINE ARGS HANDLING*/

    CommandLineArgs(&ParameterManager, argc, argv); //Define and set command line args

    if (ParameterManager.GetbyName("Path").RetrieveS().size() < 1) //Check if the path has been defined 
        return -1;

    if (ParameterManager.GetbyName("memorylimit").RetrieveI() > 0) //Update memory limit if defined in the command line args
    {
        maxMem = ParameterManager.GetbyName("memorylimit").RetrieveI(); 
    }

    if (ParameterManager.GetbyName("spacing").RetrieveF() > 0.0f) //Update the spacing if defined in the command line args
    {
        spacing = ParameterManager.GetbyName("spacing").RetrieveF();
    }

    /* GET FILE LIST*/

    try
    { 
        //Add check for ASC file command line arg %%%%%

        files = FileReader::GetFileList(ParameterManager.GetbyName("Path").RetrieveS()); //Retrieve all csv files in the given path and store a list of the files
    }
    catch(std::exception ex)
    {
        std::cout << ex.what() << "\n" << ParameterManager.GetbyName("Path").RetrieveS() << "\n"; //If an exception is thrown display the error message
        return -1;
    }

    std::cout << "Found total of " << std::to_string(files.size()) << " files to proccess." << std::endl << std::endl;

    /* CREATE A QUADTREE TO STORE THE COORDINATES*/

    QuadtreeManager<Coordinates> qm;
    qm.CreateQuadtree(files, spacing, maxMem);

    Node<Coordinates>* n = qm.Search(Coordinates(477000.5, 7103000.5));

    std::cout << n->pos.x << "," << n->pos.y << "," << n->pos.z << std::endl;

    return 0;
}
