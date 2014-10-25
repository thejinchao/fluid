#include "sph_stdafx.h"
#include "sph_neighbor_table.h"

namespace SPH
{
//-----------------------------------------------------------------------------------------------------------------
NeighborTable::NeighborTable()
	: m_pointExtraData(0)
	, m_pointCounts(0)
	, m_pointCapcity(0)
	, m_neighborDataBuf(0)
	, m_dataBufSize(0)
	, m_currNeighborCounts(0)
	, m_currPoint(0)
	, m_dataBufOffset(0)
{
}

//-----------------------------------------------------------------------------------------------------------------
NeighborTable::~NeighborTable()
{
	if(m_pointExtraData) free(m_pointExtraData);
	if(m_neighborDataBuf) free(m_neighborDataBuf);
}

//-----------------------------------------------------------------------------------------------------------------
void NeighborTable::reset(unsigned short pointCounts)
{
	int a = sizeof(PointExtraData);
	if(pointCounts>m_pointCapcity)
	{
		if(m_pointExtraData)
		{
			free(m_pointExtraData);
		}
		m_pointExtraData = (PointExtraData*)malloc(sizeof(PointExtraData)*pointCounts);
		m_pointCapcity = pointCounts;
	}

	m_pointCounts = pointCounts;
	memset(m_pointExtraData, 0, sizeof(PointExtraData)*m_pointCapcity);
	m_dataBufOffset = 0;
}

//-----------------------------------------------------------------------------------------------------------------
void NeighborTable::point_prepare(unsigned short ptIndex)
{
	m_currPoint = ptIndex;
	m_currNeighborCounts = 0;
}

//-----------------------------------------------------------------------------------------------------------------
bool NeighborTable::point_add_neighbor(unsigned short ptIndex, float distance)
{
	if(m_currNeighborCounts>=MAX_NEIGHTBOR_COUNTS) return false;

	m_currNeightborIndex[m_currNeighborCounts]=ptIndex;
	m_currNeighborDistance[m_currNeighborCounts]=distance;

	m_currNeighborCounts++;
	return true;
}

//-----------------------------------------------------------------------------------------------------------------
void NeighborTable::point_commit(void)
{
	if(m_currNeighborCounts==0) return;

	unsigned int index_size = m_currNeighborCounts*sizeof(unsigned short);
	unsigned int distance_size = m_currNeighborCounts*sizeof(float);

	//grow buf
	if(m_dataBufOffset+index_size+distance_size>m_dataBufSize) 
	{
		_growDataBuf(m_dataBufOffset+index_size+distance_size);
	}

	//set neightbor data
	m_pointExtraData[m_currPoint].neighborCounts = m_currNeighborCounts;
	m_pointExtraData[m_currPoint].neighborDataOffset = m_dataBufOffset;

	//copy index data
	memcpy(m_neighborDataBuf+m_dataBufOffset, m_currNeightborIndex, index_size);
	m_dataBufOffset += index_size;

	//copy distance data
	memcpy(m_neighborDataBuf+m_dataBufOffset, m_currNeighborDistance, distance_size);
	m_dataBufOffset += distance_size;
}

//-----------------------------------------------------------------------------------------------------------------
void NeighborTable::_growDataBuf(unsigned int need_size)
{
	unsigned int newSize = m_dataBufSize>0 ? m_dataBufSize : 1;
	while(newSize<need_size) newSize*=2;
	if(newSize<1024)newSize=1024;

	unsigned char* newBuf = (unsigned char*)malloc(newSize);
	if(m_neighborDataBuf)
	{
		memcpy(newBuf, m_neighborDataBuf, m_dataBufSize);
		free(m_neighborDataBuf);
	}
	m_neighborDataBuf = newBuf;
	m_dataBufSize = newSize;
}

//-----------------------------------------------------------------------------------------------------------------
void NeighborTable::getNeighborInfo(unsigned short ptIndex, int index, unsigned short& neighborIndex, float& neighborDistance)
{
	PointExtraData neighData = m_pointExtraData[ptIndex];

	unsigned short* indexBuf = (unsigned short*)(m_neighborDataBuf+neighData.neighborDataOffset);
	float* distanceBuf = (float*)(m_neighborDataBuf+neighData.neighborDataOffset+sizeof(unsigned short)*neighData.neighborCounts);

	neighborIndex = indexBuf[index];
	neighborDistance = distanceBuf[index];
}

}
