#include "os.h"
#include "Coordinates.h"
#include "CatchMath.h"
#include "FlowClassification.h"
#include "Quadtree.h"
#include "QuadtreeManager.h"
#include "Catchment.h"
#include "HydrologyData.h"

#include <filesystem>
#include <vector>
#include <string>

class FileWriter
{
public:
	static void WriteFiles(std::string filename, std::vector<std::string> data, bool multiplefiles);
	static void WriteCoordTree(std::string filename, QuadtreeManager<Coordinates>& quad);
	static void WriteVec2Points(std::string filename, std::vector<DischargePoint> points);
	static void WriteCoordTreeASC(std::string filename, QuadtreeManager<Coordinates>& quad);
	static void WriteFlowGeneralTreeASC(std::string filename, QuadtreeManager<FlowGeneral>& quad);
	static void WriteVecNormals2dWKT(std::string filename, QuadtreeManager<Normal>& normals);
	static void WriteStreamPaths2dWKT(std::string filename, std::vector<FlowPath>& flowpaths);
	static void WriteCatchmentPolysWKT(std::string filename, std::vector<Catchment>& catchlist);
	static void WriteStreamPathsBinary(std::string filepath, std::vector<FlowPath>& flowpaths);
	static void WriteVecNormals3dWKT(std::string filename, QuadtreeManager<Normal>& normals);
	static void WriteFlowDirection2dWKT(std::string filename, QuadtreeManager<FlowDirection>& normals);
	static void WriteCatchmentstoBinary(std::string filepath, std::vector<Catchment>& catchments);
	static void WriteTimeSeriestoCSV(std::string filepath, RainfallSeries rs);
	static void WriteTimeSeriestoCSV(std::string filepath, FlowSeries fs);
private:
	static void Write(std::string filename, std::string data);
	static void Write(std::string filename, std::vector<std::string> data);

};
