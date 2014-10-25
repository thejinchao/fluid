#include "sph_stdafx.h"
#include "sph_point_buffer.h"

namespace SPH
{
//-----------------------------------------------------------------------------------------------------------------
PointBuffer::PointBuffer()
	: mFluidBuf(0)
	, mFluidCounts(0)
	, mBufCapcity(0)
{
}

//-----------------------------------------------------------------------------------------------------------------
PointBuffer::~PointBuffer()
{
	free(mFluidBuf);
	mFluidBuf=0;
}

//-----------------------------------------------------------------------------------------------------------------
void PointBuffer::reset(unsigned int capcity)
{
	mBufCapcity = capcity;		
	if(mFluidBuf != 0)
	{
		free(mFluidBuf);
		mFluidBuf = 0;
	}

	if(mBufCapcity>0)
	{
		mFluidBuf = (Point*)malloc(mBufCapcity* sizeof(Point));
	}
	mFluidCounts = 0;
}

//-----------------------------------------------------------------------------------------------------------------
Point* PointBuffer::AddPointReuse(void)
{
	if(mFluidCounts >= mBufCapcity) 
	{
		if(mBufCapcity*2>ELEM_MAX) 
		{
			//get a random point
			unsigned int index = rand()%mFluidCounts;
			return mFluidBuf+index;
		}

		//realloc point buff
		mBufCapcity *= 2;		
		Point* new_data = (Point*)malloc(mBufCapcity* sizeof(Point));
		memcpy(new_data, mFluidBuf, mFluidCounts*sizeof(Point) );
		free(mFluidBuf);
		mFluidBuf = new_data;
	}

	//a new point
	Point* point = mFluidBuf + (mFluidCounts++);

	point->pos.set(0, 0, 0);
	point->next = 0;
	point->velocity.set(0,0,0);
	point->velocity_eval.set(0,0,0);
	point->pressure = 0;
	point->density = 0;
	point->accel.set(0,0,0);
	return point;
}

}
