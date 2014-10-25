#pragma once

namespace SPH
{

class NeighborTable
{
public:
	/** reset neighbor table */
	void reset(unsigned short pointCounts);
	/** prepare a point neighbor data */
	void point_prepare(unsigned short ptIndex);
	/** add neighbor data to current point */
	bool point_add_neighbor(unsigned short ptIndex, float distance);
	/** commit point neighbor data to data buf*/
	void point_commit(void);
	/** get point neighbor counts */
	int getNeighborCounts(unsigned short ptIndex) { return m_pointExtraData[ptIndex].neighborCounts; }
	/** get point neightbor information*/
	void getNeighborInfo(unsigned short ptIndex, int index, unsigned short& neighborIndex, float& neighborDistance);

private:
	enum {MAX_NEIGHTBOR_COUNTS=80,};

	union PointExtraData
	{
		struct
		{
			unsigned neighborDataOffset : 24;
			unsigned neighborCounts		: 8;
		};

		unsigned int neighborData;
	};

	PointExtraData* m_pointExtraData;
	unsigned int m_pointCounts;
	unsigned int m_pointCapcity;

	unsigned char* m_neighborDataBuf;	//neighbor data buf
	unsigned int m_dataBufSize;			//in bytes
	unsigned int m_dataBufOffset;		//current neighbor data buf offset

	////// temp data for current point
	unsigned short m_currPoint;
	int m_currNeighborCounts;
	unsigned short m_currNeightborIndex[MAX_NEIGHTBOR_COUNTS];
	float m_currNeighborDistance[MAX_NEIGHTBOR_COUNTS];

private:
	void _growDataBuf(unsigned int need_size);

public:
	NeighborTable();
	~NeighborTable();
};

}
