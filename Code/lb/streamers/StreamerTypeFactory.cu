
// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "lb/collisions/Collisions.h"
#include "lb/iolets/InOutLetCosine.cuh"
#include "lb/kernels/Kernels.h"
#include "lb/lattices/D3Q15.cuh"
#include "lb/lattices/Lattices.h"
#include "lb/streamers/Streamers.h"



using namespace hemelb;
using namespace hemelb::lb;



#define DmQn lattices::D3Q15_GPU



// lb/lattices/Lattice.h
__device__ void Lattice_CalculateFeq(const distribn_t& density, const double3& momentum, distribn_t* f_eq)
{
  const distribn_t density_1 = 1. / density;
  const distribn_t momentumMagnitudeSquared =
      momentum.x * momentum.x
      + momentum.y * momentum.y
      + momentum.z * momentum.z;

  for ( int j = 0; j < DmQn::NUMVECTORS; ++j )
  {
    const distribn_t mom_dot_ei =
        DmQn::CXD[j] * momentum.x
        + DmQn::CYD[j] * momentum.y
        + DmQn::CZD[j] * momentum.z;

    f_eq[j] = DmQn::EQMWEIGHTS[j]
        * (density
            - (3. / 2.) * momentumMagnitudeSquared * density_1
            + (9. / 2.) * density_1 * mom_dot_ei * mom_dot_ei
            + 3. * mom_dot_ei);
  }
}



__global__ void Normal_LBGK_SBB_Nash_StreamAndCollide(
  site_t firstIndex,
  site_t siteCount,
  distribn_t lbmParams_tau,
  distribn_t lbmParams_omega,
  const iolets::InOutLetCosineGPU* inlets,
  const iolets::InOutLetCosineGPU* outlets,
  const site_t* neighbourIndices,
  const geometry::SiteData* siteData,
  const distribn_t* fOld,
  distribn_t* fNew,
  unsigned long timeStep
)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;

  if ( i >= siteCount )
  {
    return;
  }

  site_t siteIndex = firstIndex + i;

  // initialize hydroVars
  distribn_t f[DmQn::NUMVECTORS];
  distribn_t density;
  double3 momentum;
  double3 velocity;
  distribn_t f_eq[DmQn::NUMVECTORS];
  distribn_t* f_neq = f_eq;
  distribn_t* f_post = f_eq;

  // copy fOld to local memory
  memcpy(&f[0], &fOld[siteIndex * DmQn::NUMVECTORS], DmQn::NUMVECTORS * sizeof(distribn_t));

  // Normal::DoCalculatePreCollision()
  // LBGK::DoCalculateDensityMomentumFeq()
  // Lattice::CalculateDensityMomentumFEq()
  density = 0.0;
  momentum.x = 0.0;
  momentum.y = 0.0;
  momentum.z = 0.0;

  for ( int j = 0; j < DmQn::NUMVECTORS; ++j )
  {
    density += f[j];
    momentum.x += DmQn::CXD[j] * f[j];
    momentum.y += DmQn::CYD[j] * f[j];
    momentum.z += DmQn::CZD[j] * f[j];
  }

  velocity.x = momentum.x / density;
  velocity.y = momentum.y / density;
  velocity.z = momentum.z / density;

  Lattice_CalculateFeq(density, momentum, f_eq);

  for ( int j = 0; j < DmQn::NUMVECTORS; ++j )
  {
    f_neq[j] = f[j] - f_eq[j];
  }

  // Normal::DoCollide()
  // LBGK::DoCollide()
  for ( int j = 0; j < DmQn::NUMVECTORS; ++j )
  {
    f_post[j] = f[j] + f_neq[j] * lbmParams_omega;
  }

  // perform streaming
  auto& site = siteData[siteIndex];

  for ( int j = 0; j < DmQn::NUMVECTORS; ++j )
  {
    if ( site.HasIolet(j) )
    {
      // get iolet
      auto& iolet = (site.GetSiteType() == geometry::INLET_TYPE)
        ? inlets[site.GetIoletId()]
        : outlets[site.GetIoletId()];

      // get density at the iolet
      distribn_t ioletDensity = iolet.GetDensity(timeStep);

      // compute momentum at the iolet
      distribn_t component =
          velocity.x * iolet.normal.x
          + velocity.y * iolet.normal.y
          + velocity.z * iolet.normal.z;

      double3 ioletMomentum;
      ioletMomentum.x = iolet.normal.x * component * ioletDensity;
      ioletMomentum.y = iolet.normal.y * component * ioletDensity;
      ioletMomentum.z = iolet.normal.z * component * ioletDensity;

      // compute f_eq at the iolet
      distribn_t ioletFeq[DmQn::NUMVECTORS];

      Lattice_CalculateFeq(ioletDensity, ioletMomentum, ioletFeq);

      int outIndex = siteIndex * DmQn::NUMVECTORS + DmQn::INVERSEDIRECTIONS[j];
      fNew[outIndex] = ioletFeq[DmQn::INVERSEDIRECTIONS[j]];
    }
    else if ( site.HasWall(j) )
    {
      int outIndex = siteIndex * DmQn::NUMVECTORS + DmQn::INVERSEDIRECTIONS[j];
      fNew[outIndex] = f_post[j];
    }
    else
    {
      int outIndex = neighbourIndices[siteIndex * DmQn::NUMVECTORS + j];
      fNew[outIndex] = f_post[j];
    }
  }
}



class Normal_LBGK_SBB_Nash
{
public:
  typedef typename collisions::Normal<kernels::LBGK<lattices::D3Q15>> CollisionType;
  typedef typename streamers::SimpleBounceBackDelegate<CollisionType> WallLinkType;
  typedef typename streamers::NashZerothOrderPressureDelegate<CollisionType> IoletLinkType;

  typedef typename streamers::StreamerTypeFactory<
    CollisionType,
    WallLinkType,
    IoletLinkType
  > Type;
};



template<>
void Normal_LBGK_SBB_Nash::Type::StreamAndCollideGPU(
  const site_t firstIndex,
  const site_t siteCount,
  const lb::LbmParameters* lbmParams,
  geometry::LatticeData* latDat,
  lb::SimulationState* simState
)
{
  if ( siteCount == 0 )
  {
    return;
  }

  const int BLOCK_SIZE = 256;
  const int GRID_SIZE = (siteCount + BLOCK_SIZE - 1) / BLOCK_SIZE;

  Normal_LBGK_SBB_Nash_StreamAndCollide<<<GRID_SIZE, BLOCK_SIZE>>>(
    firstIndex,
    siteCount,
    lbmParams->GetTau(),
    lbmParams->GetOmega(),
    inlets_dev,
    outlets_dev,
    latDat->GetNeighbourIndicesGPU(),
    latDat->GetSiteDataGPU(),
    latDat->GetFOldGPU(0),
    latDat->GetFNewGPU(0),
    simState->Get0IndexedTimeStep()
  );
  CUDA_SAFE_CALL(cudaGetLastError());
}
