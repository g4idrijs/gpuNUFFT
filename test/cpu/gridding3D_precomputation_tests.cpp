#include <limits.h>
#include "gridding_cpu.hpp"

#include "gtest/gtest.h"
#include "gridding_operator.hpp"

// sort algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::sort
#include <vector>       // std::vector
#include <map>

#define EPS 0.0001

IndType computeSectorCountPerDimension(IndType dim, IndType sectorWidth)
{
	return std::ceil(static_cast<DType>(dim) / sectorWidth);
}

GriddingND::Dimensions computeSectorCountPerDimension(GriddingND::Dimensions dim, IndType sectorWidth)
{
	GriddingND::Dimensions sectorDims;
	sectorDims.width = computeSectorCountPerDimension(dim.width,sectorWidth);
	sectorDims.height = computeSectorCountPerDimension(dim.height,sectorWidth);
	sectorDims.depth = computeSectorCountPerDimension(dim.depth,sectorWidth);
	return sectorDims;
}

IndType computeTotalSectorCount(GriddingND::Dimensions dim, IndType sectorWidth)
{
	return computeSectorCountPerDimension(dim,sectorWidth).count();
}

TEST(PrecomputationTest, ComputeIsotropicSectorCount) {
	IndType imageWidth = 128; 
	IndType sectorWidth = 8;

	IndType sectors = computeSectorCountPerDimension(imageWidth,sectorWidth);
	EXPECT_EQ(16,sectors);

	imageWidth = 124; 
	sectorWidth = 8;
    sectors = computeSectorCountPerDimension(imageWidth,sectorWidth);
	EXPECT_EQ(16,sectors);

	imageWidth = 7; 
	sectorWidth = 8;
    sectors = computeSectorCountPerDimension(imageWidth,sectorWidth);
	EXPECT_EQ(1,sectors);

	imageWidth = 120; 
	sectorWidth = 8;
    sectors = computeSectorCountPerDimension(imageWidth,sectorWidth);
	EXPECT_EQ(15,sectors);
}

TEST(PrecomputationTest, ComputeIsotropicSectorDim) {
	IndType imageWidth = 128; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

    GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	IndType sectors = computeSectorCountPerDimension(gridDim.width, sectorWidth);
	IndType sectorDim = computeTotalSectorCount(gridDim,sectorWidth);
	EXPECT_EQ(16*osr,sectors);
	
	IndType expected = 16*16*16*osr*osr*osr;
	EXPECT_EQ(expected,sectorDim);	

	GriddingND::Dimensions sectorDims = computeSectorCountPerDimension(gridDim,sectorWidth);

	EXPECT_EQ(expected,sectorDims.count());
}

TEST(PrecomputationTest, ComputeAnisotropicSectorDim) {
	IndType imageWidth = 128; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

    GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth);
	gridDim.height = (IndType)(imageWidth);
	gridDim.depth = (IndType)((imageWidth-16));
	
	std::cout << " dimensions before: w: " << gridDim.width << " h: " << gridDim.height << " d: " << gridDim.depth << std::endl;
	gridDim = gridDim * osr;
	std::cout << " dimensions scaled: w: " << gridDim.width << " h: " << gridDim.height << " d: " << gridDim.depth << std::endl;

	IndType sectorDim = computeTotalSectorCount(gridDim,sectorWidth);
	 
	IndType expected = 16*16*14*osr*osr*osr;
	EXPECT_EQ(expected,sectorDim);	

	GriddingND::Dimensions sectorDims = computeSectorCountPerDimension(gridDim,sectorWidth);

	EXPECT_EQ(expected,sectorDims.count());
}

TEST(PrecomputationTest, ComputeSectorRanges) {
	IndType imageWidth = 128; 
	DType osr = 1.0;
	IndType sectorWidth = 8;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	GriddingND::Dimensions sectorDims = computeSectorCountPerDimension(gridDim,sectorWidth);

	DType expected[17] = {-0.5000,-0.4375,-0.3750,-0.3125,-0.2500,-0.1875,-0.1250,-0.0625,0,0.0625,0.1250,0.1875,0.2500,0.3125,0.3750,0.4375,0.5000};

	DType* sectorRange = (DType*)malloc((sectorDims.width +1) * sizeof(DType));
	//linspace in range from -0.5 to 0.5
	for (int i=0; i <= sectorDims.width; i++)
	{
		sectorRange[i] = -0.5 + i*(static_cast<DType>(1.0) / (sectorDims.width));
		printf("%5.4f ",sectorRange[i]);
		EXPECT_NEAR(sectorRange[i],expected[i],EPS);
	}
	std::cout << std::endl;

	free(sectorRange);
}

TEST(PrecomputationTest, AssignSectors1D) {
	IndType imageWidth = 16; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

	const IndType coordCnt = 6;
	
	// Coords as StructureOfArrays
	// i.e. first x-vals, then y-vals and z-vals
	DType coords[coordCnt] = {-0.5,-0.3,-0.1, 0.1, 0.3, 0.5};//x

	GriddingND::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = coordCnt;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	GriddingND::Dimensions sectorDims = computeSectorCountPerDimension(gridDim,sectorWidth);

	DType expected[4] = {-0.5000,-0.16666,0.16666,0.5000};

	DType* sectorRange = (DType*)malloc((sectorDims.width +1) * sizeof(DType));
	//linspace in range from -0.5 to 0.5
	for (int i=0; i <= sectorDims.width; i++)
	{
		sectorRange[i] = -0.5 + i*(static_cast<DType>(1.0) / (sectorDims.width));
		printf("%5.4f ",sectorRange[i]);
		EXPECT_NEAR(sectorRange[i],expected[i],EPS);
	}
	std::cout << std::endl;
	
	IndType expectedSec[6] = {0,0,1,1,2,2};

	for (int cCnt = 0; cCnt < coordCnt; cCnt++)
	{
		DType x = kSpaceData.data[cCnt];
		std::cout << "processing x var: " << x << std::endl;
		IndType sector = std::floor(static_cast<float>(x + 0.5) * sectorDims.width);
		if (sector == sectorDims.width) 
			sector--;
		std::cout << "into sector : " << sector << std::endl;
		EXPECT_EQ(expectedSec[cCnt],sector);
	}

	free(sectorRange);
}

TEST(PrecomputationTest, AssignSectors1DOnBorders) {
	IndType imageWidth = 16; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

	const IndType coordCnt = 4;
	
	// Coords as StructureOfArrays
	// i.e. first x-vals, then y-vals and z-vals
	DType coords[coordCnt] = {-0.5,-0.1666,0.1666,0.5};//x

	GriddingND::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = coordCnt;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	GriddingND::Dimensions sectorDims= computeSectorCountPerDimension(gridDim,sectorWidth);

	DType expected[4] = {-0.5000,-0.16666,0.16666,0.5000};

	DType* sectorRange = (DType*)malloc((sectorDims.width +1) * sizeof(DType));
	//linspace in range from -0.5 to 0.5
	for (int i=0; i <= sectorDims.width; i++)
	{
		sectorRange[i] = -0.5 + i*(static_cast<DType>(1.0) / (sectorDims.width));
		printf("%5.4f ",sectorRange[i]);
		EXPECT_NEAR(sectorRange[i],expected[i],EPS);
	}
	std::cout << std::endl;
	
	IndType expectedSec[4] = {0,1,2,2};

	for (int cCnt = 0; cCnt < coordCnt; cCnt++)
	{
		DType x = kSpaceData.data[cCnt];
		std::cout << "processing x var: " << x << std::endl;
		IndType sector = std::round(static_cast<float>(x + 0.5) * sectorDims.width);
		if (sector == sectorDims.width) 
			sector--;
		std::cout << "into sector : " << sector << std::endl;
		EXPECT_EQ(expectedSec[cCnt],sector);
	}

	free(sectorRange);
}


TEST(PrecomputationTest, AssignSectors2D) {
	IndType imageWidth = 16; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

	const IndType coordCnt = 6;
	
	// Coords as StructureOfArrays
	// i.e. first x-vals, then y-vals and z-vals
	DType coords[coordCnt*2] = {-0.5,-0.3,-0.1, 0.1, 0.3, 0.5,//x
	                            -0.5,-0.5,   0,   0, 0.5, 0.45};//y

	GriddingND::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = coordCnt;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	GriddingND::Dimensions sectorDims= computeSectorCountPerDimension(gridDim,sectorWidth);

	DType expected[4] = {-0.5000,-0.16666,0.16666,0.5000};

	DType* sectorRange = (DType*)malloc((sectorDims.width +1) * sizeof(DType));
	//linspace in range from -0.5 to 0.5
	for (int i=0; i <= sectorDims.width; i++)
	{
		sectorRange[i] = -0.5 + i*(static_cast<DType>(1.0) / (sectorDims.width));
		EXPECT_NEAR(sectorRange[i],expected[i],EPS);
	}
	std::cout << std::endl;
	
	IndType expectedSec[6] = {0,0,4,4,8,8};

	for (int cCnt = 0; cCnt < coordCnt; cCnt++)
	{
		DType2 coord;
		coord.x = kSpaceData.data[cCnt];
		coord.y = kSpaceData.data[cCnt + kSpaceData.count()];
		
		std::cout << "processing x var: " << coord.x << " y: " << coord.y << std::endl;

		IndType x_sector = std::floor(static_cast<float>(coord.x + 0.5) * sectorDims.width);
		if (x_sector == sectorDims.width) 
			x_sector--;

		IndType y_sector = std::floor(static_cast<float>(coord.y + 0.5) * sectorDims.height);
		if (y_sector == sectorDims.height) 
			y_sector--;

		std::cout << "into sector x: " << x_sector << " y: " << y_sector << std::endl;
		EXPECT_EQ(expectedSec[cCnt],x_sector + y_sector * sectorDims.height);
	}

	free(sectorRange);
}

IndType computeSectorMapping(DType coord, IndType sectorCount)
{
	IndType sector = std::floor(static_cast<DType>(coord + 0.5) * sectorCount);
	if (sector == sectorCount) 
		sector--;
	return sector;
}

IndType3 computeSectorMapping(DType3 coord, GriddingND::Dimensions sectorDims)
{
	IndType3 sector;
	sector.x = computeSectorMapping(coord.x,sectorDims.width);
	sector.y  = computeSectorMapping(coord.y,sectorDims.height);
	sector.z  = computeSectorMapping(coord.z,sectorDims.depth);
	return sector;
}

IndType2 computeSectorMapping(DType2 coord, GriddingND::Dimensions sectorDims)
{
	IndType2 sector;
	sector.x = computeSectorMapping(coord.x,sectorDims.width);
	sector.y  = computeSectorMapping(coord.y,sectorDims.height);
	return sector;
}

IndType computeXYZ2Lin(IndType x, IndType y, IndType z, GriddingND::Dimensions dim)
{
	return x + dim.height * (y + dim.depth * z);
}

IndType computeInd32Lin(IndType3 sector, GriddingND::Dimensions dim)
{
	return sector.x + dim.height * (sector.y + dim.depth * sector.z);
}

TEST(PrecomputationTest, AssignSectors3D) {
	IndType imageWidth = 16; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

	const IndType coordCnt = 6;
	
	// Coords as StructureOfArrays
	// i.e. first x-vals, then y-vals and z-vals
	DType coords[coordCnt*3] = {-0.5,-0.3,-0.1, 0.1, 0.3, 0.5,//x
	                            -0.5,-0.5,   0,   0, 0.5, 0.45,//y
	                            -0.33,-0.16666,   0,   0, -0.23, 0.45};//z

	GriddingND::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = coordCnt;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	GriddingND::Dimensions sectorDims= computeSectorCountPerDimension(gridDim,sectorWidth);

	DType expected[4] = {-0.5000,-0.16666,0.16666,0.5000};

	DType* sectorRange = (DType*)malloc((sectorDims.width +1) * sizeof(DType));
	//linspace in range from -0.5 to 0.5
	for (int i=0; i <= sectorDims.width; i++)
	{
		sectorRange[i] = -0.5 + i*(static_cast<DType>(1.0) / (sectorDims.width));
		EXPECT_NEAR(sectorRange[i],expected[i],EPS);
	}
	std::cout << std::endl;
	
	IndType expectedSec[6] = {0,9,13,13,8,26};

	for (int cCnt = 0; cCnt < coordCnt; cCnt++)
	{
		DType3 coord;
		coord.x = kSpaceData.data[cCnt];
		coord.y = kSpaceData.data[cCnt + kSpaceData.count()];
		coord.z = kSpaceData.data[cCnt + 2*kSpaceData.count()];
		
		std::cout << "processing x var: " << coord.x << " y: " << coord.y << " z: " << coord.z  << std::endl;

		IndType x_sector = computeSectorMapping(coord.x,sectorDims.width);
		IndType y_sector = computeSectorMapping(coord.y,sectorDims.height);
		IndType z_sector = computeSectorMapping(coord.z,sectorDims.depth);

		std::cout << "into sector x: " << x_sector << " y: " << y_sector << " z: " << z_sector << std::endl;
		EXPECT_EQ(expectedSec[cCnt],computeXYZ2Lin(x_sector,y_sector,z_sector,sectorDims));

		IndType3 mappedSectors = computeSectorMapping(coord,sectorDims);
		EXPECT_EQ(expectedSec[cCnt],computeInd32Lin(mappedSectors,sectorDims));
	}

	free(sectorRange);
}

bool pairComp (std::pair<IndType,IndType> i,std::pair<IndType,IndType> j) 
{ 
	return (i.second < j.second); 
}

std::vector<GriddingND::IndPair> sortVector(GriddingND::Array<IndType> assignedSectors)
{
	std::vector<GriddingND::IndPair> secVector;
	
	for (IndType i=0; i< assignedSectors.count(); i++)
	  secVector.push_back(GriddingND::IndPair(i,assignedSectors.data[i]));

	// using function as comp
	std::sort (secVector.begin(), secVector.end());

	return secVector;
}

TEST(PrecomputationTest, TestIndexSorting) 
{
  IndType assSectors[6] = {0,9,13,13,8,26};
  IndType expectedSectors[6] = {0,8,9,13,13,26};

  GriddingND::Array<IndType> assignedSectors;
  assignedSectors.data = assSectors;
  assignedSectors.dim.length = 6;

  std::vector<GriddingND::IndPair> secVector = sortVector(assignedSectors);

  // print out content:
  std::cout << "vector contains:";
  for (std::vector<GriddingND::IndPair>::iterator it=secVector.begin(); it!=secVector.end(); ++it)
    std::cout << " " << it->second << " (" << it->first << ") ";
  std::cout << '\n';

  GriddingND::IndPair* sortedArray = &secVector[0];

  //print indices for reselect
  for (IndType i=0; i<6;i++)
  {
	  std::cout << sortedArray[i].first ;
	  EXPECT_EQ(sortedArray[i].second, expectedSectors[i]);
  }
  std::cout << std::endl;
}

TEST(PrecomputationTest, AssignSectors3DSorted) {
	IndType imageWidth = 16; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

	const IndType coordCnt = 6;
	
	// Coords as StructureOfArrays
	// i.e. first x-vals, then y-vals and z-vals
	DType coords[coordCnt*3] = {-0.5,-0.3,-0.1, 0.1, 0.3, 0.5,//x
	                            -0.5,-0.5,   0,   0, 0.5, 0.45,//y
	                            -0.33,-0.16666,   0,   0, -0.23, 0.45};//z

	GriddingND::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = coordCnt;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	GriddingND::Dimensions sectorDims= computeSectorCountPerDimension(gridDim,sectorWidth);

	IndType expectedSec[6] = {0,9,13,13,8,26};

	GriddingND::Array<IndType> assignedSectors;
    assignedSectors.data = (IndType*)malloc(coordCnt * sizeof(IndType));
    assignedSectors.dim.length = coordCnt;

	for (int cCnt = 0; cCnt < coordCnt; cCnt++)
	{
		DType3 coord;
		coord.x = kSpaceData.data[cCnt];
		coord.y = kSpaceData.data[cCnt + kSpaceData.count()];
		coord.z = kSpaceData.data[cCnt + 2*kSpaceData.count()];

		IndType3 mappedSectors = computeSectorMapping(coord,sectorDims);

		IndType sector = computeInd32Lin(mappedSectors,sectorDims);
		assignedSectors.data[cCnt] = sector;
		EXPECT_EQ(expectedSec[cCnt],sector);
	}

	IndType expectedSecSorted[6] = {0,8,9,13,13,26};
	IndType expectedSecIndexSorted[6] = {0,4,1,2,3,5};

    std::vector<GriddingND::IndPair> secVector = sortVector(assignedSectors);
	
	DType coords_sorted[coordCnt*3];

	for (int i=0; i<assignedSectors.count();i++)
	{
		//compare index
		EXPECT_EQ(expectedSecIndexSorted[i],secVector[i].first);
		EXPECT_EQ(expectedSecSorted[i],secVector[i].second);
		coords_sorted[i] = kSpaceData.data[secVector[i].first];
		coords_sorted[i + 1*coordCnt] = kSpaceData.data[secVector[i].first + 1*coordCnt];
		coords_sorted[i + 2*coordCnt] = kSpaceData.data[secVector[i].first + 2*coordCnt];
	}

	for (int i=0;i<kSpaceData.count();i++)
	{
		std::cout << " x: "  << coords_sorted[i] << " y: " << coords_sorted[i+ 1*coordCnt] << " z:" << coords_sorted[i+ 2*coordCnt] << std::endl;
	}

	free(assignedSectors.data);
}

TEST(PrecomputationTest, ComputeDataIndices) {
	IndType imageWidth = 16; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

	const IndType coordCnt = 6;
	
	// Coords as StructureOfArrays
	// i.e. first x-vals, then y-vals and z-vals
	DType coords[coordCnt*3] = {-0.5,-0.3,-0.1, 0.1, 0.3, 0.5,//x
	                            -0.5,-0.5,   0,   0, 0.5, 0.45,//y
	                            -0.33,-0.16666,   0,   0, -0.23, 0.45};//z

	GriddingND::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = coordCnt;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);

	GriddingND::Dimensions sectorDims= computeSectorCountPerDimension(gridDim,sectorWidth);

	IndType expectedSec[6] = {0,9,13,13,8,26};

	GriddingND::Array<IndType> assignedSectors;
    assignedSectors.data = (IndType*)malloc(coordCnt * sizeof(IndType));
    assignedSectors.dim.length = coordCnt;

	for (int cCnt = 0; cCnt < coordCnt; cCnt++)
	{
		DType3 coord;
		coord.x = kSpaceData.data[cCnt];
		coord.y = kSpaceData.data[cCnt + kSpaceData.count()];
		coord.z = kSpaceData.data[cCnt + 2*kSpaceData.count()];

		IndType3 mappedSectors = computeSectorMapping(coord,sectorDims);

		IndType sector = computeInd32Lin(mappedSectors,sectorDims);
		assignedSectors.data[cCnt] = sector;
		EXPECT_EQ(expectedSec[cCnt],sector);
	}

	IndType expectedSecSorted[6] = {0,8,9,13,13,26};
	IndType expectedSecIndexSorted[6] = {0,4,1,2,3,5};

    std::vector<GriddingND::IndPair> secVector = sortVector(assignedSectors);

	DType coords_sorted[coordCnt*3];
	
	for (int i=0; i<coordCnt;i++)
	{
		//compare index
		EXPECT_EQ(expectedSecIndexSorted[i],secVector[i].first);
		EXPECT_EQ(expectedSecSorted[i],secVector[i].second);
		coords_sorted[i] = kSpaceData.data[secVector[i].first];
		coords_sorted[i + 1*coordCnt] = kSpaceData.data[secVector[i].first + 1*coordCnt];
		coords_sorted[i + 2*coordCnt] = kSpaceData.data[secVector[i].first + 2*coordCnt];
	}

	IndType cnt = 0;
	std::vector<IndType> dataIndices;

	IndType sectorDataCount[29] = {0,1,1,1,1,1,1,1,1,2,3,3,3,3,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6};
	
	dataIndices.push_back(0);
	for (int i=0; i<=sectorDims.count(); i++)
	{	
		while (cnt < coordCnt && i == secVector[cnt].second)
			cnt++;
		
		dataIndices.push_back(cnt);
		EXPECT_EQ(sectorDataCount[i+1],cnt);
	}

	for (int i=0; i<dataIndices.size(); i++)
	{
		std::cout << dataIndices.at(i) << " " ;
	}
	std::cout << std::endl;
	free(assignedSectors.data);
}

TEST(PrecomputationTest, ComputeSectorCenters) {
	IndType imageWidth = 16; 
	DType osr = 1.5;
	IndType sectorWidth = 8;

	GriddingND::Dimensions gridDim;
	gridDim.width = (IndType)(imageWidth * osr);
	gridDim.height = (IndType)(imageWidth * osr);
	gridDim.depth = (IndType)(imageWidth * osr);
	
	GriddingND::Dimensions sectorDims= computeSectorCountPerDimension(gridDim,sectorWidth);

	GriddingND::Array<IndType3> sectorCenters; 
	sectorCenters.data = (IndType3*)malloc(sectorDims.count() * sizeof(IndType3));
	sectorCenters.dim.length = sectorDims.count();

	for (IndType z=0;z<sectorDims.depth; z++)
		for (IndType y=0;y<sectorDims.height;y++)
			for (IndType x=0;x<sectorDims.width;x++)
			{
				IndType3 center;
				center.x = x*sectorWidth +  std::floor(static_cast<DType>(sectorWidth) / (DType)2.0);
				center.y = y*sectorWidth +  std::floor(static_cast<DType>(sectorWidth) / (DType)2.0);
				center.z = z*sectorWidth +  std::floor(static_cast<DType>(sectorWidth) / (DType)2.0);
				sectorCenters.data[computeXYZ2Lin(x,y,z,sectorDims)] = center;
			}

	for (int i=0; i<sectorDims.count(); i++)
	{
		std::cout << " x: " << sectorCenters.data[i].x << " y: " << sectorCenters.data[i].y << " z: " << sectorCenters.data[i].z << std::endl;
	}

	EXPECT_EQ(IndType3(4,4,4).x,sectorCenters.data[0].x);
	EXPECT_EQ(IndType3(4,4,4).y,sectorCenters.data[0].y);
	EXPECT_EQ(IndType3(4,4,4).z,sectorCenters.data[0].z);

	free(sectorCenters.data);
}