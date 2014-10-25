#pragma once

#include "sph_math.h"

namespace SPH
{

struct GridCell
{
	fBox3 range;
	unsigned int counts;
};

}
