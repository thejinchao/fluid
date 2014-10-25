#pragma once

#include "sph_math.h"

namespace SPH
{

class PointBuffer;

class GridContainer
{
public:
	// Spatial Subdivision
	void init(const fBox3& box, float sim_scale, float cell_size, float border);
	void insertParticles(PointBuffer* pointBuffer);	
	void findCells(const fVector3& p, float radius, int* gridCell);
	int findCell(const fVector3& p);
	int getGridData(int gridIndex);
	
	const iVector3* getGridRes(void) const { return &m_GridRes; }
	const fVector3* getGridMin(void) const { return &m_GridMin; }
	const fVector3* getGridMax(void) const { return &m_GridMax; }
	const fVector3* getGridSize(void) const { return &m_GridSize; }

	int getGridCellIndex(float px, float py, float pz);

private:
	// Spatial Grid
	std::vector< int >	m_gridData;
	fVector3			m_GridMin;				// volume of grid (may not match domain volume exactly)
	fVector3			m_GridMax;
	iVector3			m_GridRes;				// resolution in each axis
	fVector3			m_GridSize;				// physical size in each axis
	fVector3			m_GridDelta;
	float				m_GridCellsize;

public:
	GridContainer();
	~GridContainer();
};


}
