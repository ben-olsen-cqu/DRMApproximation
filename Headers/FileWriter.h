#include "os.h"
#include "Coordinates.h"
#include "CatchMath.h"
#include "FlowClassification.h"
#include "Quadtree.h"
#include "QuadtreeManager.h"

#include <filesystem>
#include <vector>
#include <string>

class FileWriter
{
public:
	static void WriteFiles(std::string filename, std::vector<std::string> data, bool multiplefiles);
	static void WriteCoordTree(std::string filename, QuadtreeManager<Coordinates>& quad);
	static void WriteCoordTreeASC(std::string filename, QuadtreeManager<Coordinates>& quad);
	static void WriteFlowGeneralTreeASC(std::string filename, QuadtreeManager<FlowGeneral>& quad);
    static void WriteVecNormals2d(std::string filename, Quadtree<Normal>* normals);
	static void WriteVecNormals3d(std::string filename, Quadtree<Normal>* normals);
	static void WriteVecNormals2dWKT(std::string filename, QuadtreeManager<Normal>& normals);
	static void WriteStreamPaths2dWKT(std::string filename, std::vector<FlowPath>& flowpaths);
	static void WriteVecNormals3dWKT(std::string filename, QuadtreeManager<Normal>& normals);
	static void WriteFlowDirection2dWKT(std::string filename, QuadtreeManager<FlowDirection>& normals);
private:
	static void Write(std::string filename, std::string data);
	static void Write(std::string filename, std::vector<std::string> data);

};
