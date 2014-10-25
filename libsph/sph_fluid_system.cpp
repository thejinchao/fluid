#include "sph_stdafx.h"
#include "sph_fluid_system.h"

namespace SPH
{

//-----------------------------------------------------------------------------------------------------------------
FluidSystem::FluidSystem()
{
	m_unitScale			= 0.004f;			// 尺寸单位
	m_viscosity			= 1.0f;				// 粘度
	m_restDensity		= 1000.f;			// 密度
	m_pointMass			= 0.0004f;			// 粒子质量
	m_gasConstantK		= 1.0f;				// 理想气体方程常量
	m_smoothRadius		= 0.01f;			// 光滑核半径

	m_boundartStiffness = 10000.f;
	m_boundaryDampening = 256.f;
	m_speedLimiting		= 200.f;

	//Poly6 Kernel
	m_kernelPoly6 = 315.0f/(64.0f * 3.141592f * pow(m_smoothRadius, 9));
	//Spiky Kernel
	m_kernelSpiky = -45.0f/(3.141592f * pow(m_smoothRadius, 6));
	//Viscosity Kernel
	m_kernelViscosity = 45.0f/(3.141592f * pow(m_smoothRadius, 6));
}

//-----------------------------------------------------------------------------------------------------------------
FluidSystem::~FluidSystem()
{
}

//-----------------------------------------------------------------------------------------------------------------
void FluidSystem::_addFluidVolume(const fBox3& fluidBox, float spacing)
{
	float cx = (fluidBox.max.x+fluidBox.min.x)/2.f;
	float cy = (fluidBox.max.y+fluidBox.min.y)/2.f;
	float cz = (fluidBox.max.z+fluidBox.min.z)/2.f;

	for(float z=fluidBox.max.z; z>=fluidBox.min.z; z-=spacing)
	{
		for(float y=fluidBox.min.y; y<=fluidBox.max.y; y+=spacing)
		{	
			for(float x=fluidBox.min.x; x<=fluidBox.max.x; x+=spacing) 
			{
				Point* p = m_pointBuffer.AddPointReuse();

				p->pos.set(x, y, z);
			}
		}
	}	
}

//-----------------------------------------------------------------------------------------------------------------
void FluidSystem::tick(void)
{
	m_gridContainer.insertParticles(&m_pointBuffer);
	_computePressure();
	_computeForce();		
	_advance();
}

//-----------------------------------------------------------------------------------------------------------------
void FluidSystem::_init(unsigned short maxPointCounts, const fBox3& wallBox, const fBox3& initFluidBox, const fVector3& gravity)
{
	m_pointBuffer.reset(maxPointCounts);

	m_sphWallBox = wallBox;
	m_gravityDir = gravity;

	// Create the particles
	float pointDistance	= pow(m_pointMass/m_restDensity, 1.f/3.f); //粒子间距
	_addFluidVolume(initFluidBox, pointDistance/m_unitScale);

	// Setup grid Grid cell size (2r)	
	m_gridContainer.init(wallBox, m_unitScale, m_smoothRadius*2.f, 1.0);
}

//-----------------------------------------------------------------------------------------------------------------
void FluidSystem::_computePressure(void)
{
	//h^2
	float h2 = m_smoothRadius*m_smoothRadius;

	//reset neightbor table
	m_neighborTable.reset(m_pointBuffer.size());

	for(unsigned int i=0; i<m_pointBuffer.size(); i++)
	{
		Point* pi = m_pointBuffer.get(i);

		float sum = 0.f;
		m_neighborTable.point_prepare(i);

		int gridCell[8];
		m_gridContainer.findCells(pi->pos, m_smoothRadius/m_unitScale, gridCell);

		for(int cell=0; cell < 8; cell++) 
		{
			if(gridCell[cell] == -1) continue;

			int pndx = m_gridContainer.getGridData(gridCell[cell]);
			while(pndx != -1)
			{					
				Point* pj = m_pointBuffer.get(pndx);
				if(pj == pi)
				{
					sum += pow(h2, 3.f);  //self
				}
				else
				{
					fVector3 pi_pj = (pi->pos - pj->pos)*m_unitScale;
					float r2 = pi_pj.len_sq();
					if (h2 > r2) 
					{
						float h2_r2 =  h2 - r2;
						sum += pow(h2_r2, 3.f);  //(h^2-r^2)^3

						if(!m_neighborTable.point_add_neighbor(pndx, sqrt(r2)))
						{
							goto NEIGHBOR_FULL;
						}
					}
				}
				pndx = pj->next;
			}

		}

NEIGHBOR_FULL:
		m_neighborTable.point_commit();

		//m_kernelPoly6 = 315.0f/(64.0f * 3.141592f * h^9);
		pi->density = m_kernelPoly6*m_pointMass*sum;
		pi->pressure = (pi->density - m_restDensity)*m_gasConstantK;		
	}
}

//-----------------------------------------------------------------------------------------------------------------
void FluidSystem::_computeForce(void)
{
	float h2 = m_smoothRadius*m_smoothRadius;

	for(unsigned int i=0; i<m_pointBuffer.size(); i++)
	{
		Point* pi = m_pointBuffer.get(i);

		fVector3 accel_sum;
		int neighborCounts = m_neighborTable.getNeighborCounts(i);

		for(int j=0; j <neighborCounts; j++)
		{
			unsigned short neighborIndex;
			float r;
			m_neighborTable.getNeighborInfo(i, j, neighborIndex, r);

			Point* pj = m_pointBuffer.get(neighborIndex);
			//r(i)-r(j)
			fVector3 ri_rj = (pi->pos - pj->pos)*m_unitScale;
			//h-r
			float h_r = m_smoothRadius - r;
			//h^2-r^2
			float h2_r2 = h2 - r*r;

			//F_Pressure
			//m_kernelSpiky = -45.0f/(3.141592f * h^6);			
			float pterm = -m_pointMass*m_kernelSpiky*h_r*h_r*(pi->pressure+pj->pressure)/(2.f * pi->density * pj->density);
			accel_sum += ri_rj*pterm/r;

			//F_Viscosity
			//m_kernelViscosity = 45.0f/(3.141592f * h^6);
			float vterm = m_kernelViscosity * m_viscosity * h_r * m_pointMass/(pi->density * pj->density);
			accel_sum += (pj->velocity_eval - pi->velocity_eval)*vterm;
		}

		pi->accel = accel_sum;
	}
}

//-----------------------------------------------------------------------------------------------------------------
void FluidSystem::_advance(void)
{
	//fixed delta time per frame
	float deltaTime = 0.003f;

	float SL2 = m_speedLimiting*m_speedLimiting;

	for(unsigned int i=0; i<m_pointBuffer.size(); i++)
	{
		Point* p = m_pointBuffer.get(i);

		// Compute Acceleration		
		fVector3 accel = p->accel;

		// Velocity limiting 
		float accel_2 = accel.len_sq();
		if(accel_2 > SL2)
		{
			accel *= m_speedLimiting/sqrt(accel_2);
		}		

		// Boundary Conditions

		// Z-axis walls
		float diff = 2 * m_unitScale - (p->pos.z - m_sphWallBox.min.z)*m_unitScale;
		if (diff > 0.f ) 
		{			
			fVector3 norm(0, 0, 1);
			float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot ( p->velocity_eval );
			accel.x += adj * norm.x; 
			accel.y += adj * norm.y; 
			accel.z += adj * norm.z;
		}		

		diff = 2 * m_unitScale - (m_sphWallBox.max.z - p->pos.z)*m_unitScale;
		if (diff > 0.f) 
		{
			fVector3 norm( 0, 0, -1);
			float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot ( p->velocity_eval );
			accel.x += adj * norm.x; 
			accel.y += adj * norm.y; 
			accel.z += adj * norm.z;
		}

		// X-axis walls
		diff = 2 * m_unitScale - (p->pos.x - m_sphWallBox.min.x)*m_unitScale;	
		if (diff > 0.f ) 
		{
			fVector3 norm(1, 0, 0);
			float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot ( p->velocity_eval ) ;
			accel.x += adj * norm.x; 
			accel.y += adj * norm.y; 
			accel.z += adj * norm.z;					
		}

		diff = 2 * m_unitScale - (m_sphWallBox.max.x - p->pos.x)*m_unitScale;	
		if (diff > 0.f) 
		{
			fVector3 norm(-1, 0, 0);
			float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot ( p->velocity_eval );
			accel.x += adj * norm.x; 
			accel.y += adj * norm.y; 
			accel.z += adj * norm.z;
		}

		// Y-axis walls
		diff = 2 * m_unitScale - ( p->pos.y - m_sphWallBox.min.y )*m_unitScale;			
		if (diff > 0.f) 
		{
			fVector3 norm(0, 1, 0);
			float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot ( p->velocity_eval );
			accel.x += adj * norm.x; 
			accel.y += adj * norm.y; 
			accel.z += adj * norm.z;
		}
		diff = 2 * m_unitScale - ( m_sphWallBox.max.y - p->pos.y )*m_unitScale;
		if (diff > 0.f) 
		{
			fVector3 norm(0, -1, 0);
			float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot ( p->velocity_eval );
			accel.x += adj * norm.x; 
			accel.y += adj * norm.y; 
			accel.z += adj * norm.z;
		}

		// Plane gravity
		accel += m_gravityDir;

		// Leapfrog Integration ----------------------------
		fVector3 vnext = p->velocity + accel*deltaTime;			// v(t+1/2) = v(t-1/2) + a(t) dt			
		p->velocity_eval = (p->velocity + vnext)*0.5f;				// v(t+1) = [v(t-1/2) + v(t+1/2)] * 0.5		used to compute forces later
		p->velocity = vnext;
		p->pos += vnext*deltaTime/m_unitScale;		// p(t+1) = p(t) + v(t+1/2) dt
	}
}

}

//-----------------------------------------------------------------------------------------------------------------
SPH::System * getSPHSystem(void)
{
	static SPH::FluidSystem s_theSystem;
	return &s_theSystem;
}
