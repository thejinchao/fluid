#pragma once

#include "sph_math.h"	

namespace SPH
{
	/** Fluid Point
	*/
	struct Point
	{
	public:
		fVector3		pos;		//位置
		float			density;	//密度
		float			pressure;	//压力
		fVector3		accel;		//加速度

		fVector3		velocity;			
		fVector3		velocity_eval;		

		int				next;
	};

	/** point buffer
	*/
	class PointBuffer
	{
	public:
		void reset(unsigned int capcity);
		unsigned int size(void) const { return mFluidCounts; }
		Point* get(unsigned int index) { return mFluidBuf+index; }
		const Point* get(unsigned int index) const { return mFluidBuf+index; }
		Point* AddPointReuse(void);

	private:
		enum { ELEM_MAX=4096, };

		Point* mFluidBuf;
		unsigned int mFluidCounts;
		unsigned int mBufCapcity;

	public:
		PointBuffer();
		virtual ~PointBuffer();
	};
}
