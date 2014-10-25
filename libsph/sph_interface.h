#ifndef __LIB_SPH_INTERFACE_H__
#define __LIB_SPH_INTERFACE_H__

#ifdef LIBSPH_EXPORTS
	#define SPH_API  __declspec(dllexport)
#else
	#define SPH_API  __declspec(dllimport)
#endif


namespace SPH
{

struct float_3
{
	float x, y, z;
};

class System
{
public:
	virtual void init(unsigned short maxPointCounts, 
		const float_3* wallBox_min, const float_3* wallBox_max, 
		const float_3* initFluidBox_min, const float_3* initFluidBox_max, 
		const float_3* gravity) = 0;

	virtual unsigned int getPointStride(void) const = 0;
	virtual unsigned int getPointCounts(void) const = 0;
	virtual const float_3* getPointBuf(void) const = 0;
	virtual void tick(void) = 0;
};

}

extern "C"
{

/** Get the sigleton SPH System point
*/
SPH_API SPH::System * getSPHSystem(void);

};

#endif
