#ifndef HEMELB_VIS_RAYTRACER_H
#define HEMELB_VIS_RAYTRACER_H

#include "constants.h"
#include "geometry/LatticeData.h"
#include "topology/NetworkTopology.h"

#include "vis/DomainStats.h"
#include "vis/Screen.h"
#include "vis/Viewpoint.h"
#include "vis/VisSettings.h"

namespace hemelb
{
  namespace vis
  {
    class RayTracer
    {
      public:
        // Constructor and destructor do all the usual stuff.
        RayTracer(const topology::NetworkTopology * iNetworkTopology,
                  const geometry::LatticeData* iLatDat,
                  DomainStats* iDomainStats,
                  Screen* iScreen,
                  Viewpoint* iViewpoint,
                  VisSettings* iVisSettings);
        ~RayTracer();

        // Method to update the voxel corresponding to site i with its
        // newly calculated density, velocity and stress.
        void UpdateClusterVoxel(const int &i,
                                const float &density,
                                const float &velocity,
                                const float &stress);

        // Render the current state into an image.
        void Render(const lb::StressTypes iLbmStressType);

      private:

        struct Ray
        {
            float Direction[3];
            float InverseDirection[3];
            float Length;

            float VelocityColour[3];
            float StressColour[3];
            float Stress;
            float Density;
            float MinT;
        };

        struct AABB
        {
            float acc_1, acc_2, acc_3, acc_4, acc_5, acc_6;
        };

        struct Cluster
        {
            float minmax_x[2], minmax_y[2], minmax_z[2];

            float x[3];

            unsigned short int blocks_x, blocks_y, blocks_z;
            unsigned short int block_min[3];
        };

        // Some sort of coordinates.
        struct BlockLocation
        {
            short int i, j, k;
        };

        void rtUpdateRayData(float *flow_field,
                             float ray_t,
                             float ray_segment,
                             Ray *bCurrentRay,
                             void(*ColourPalette)(float value, float col[]),
                             const lb::StressTypes iLbmStressType);

        void rtTraverseVoxels(float block_min[],
                              float block_x[],
                              float voxel_flow_field[],
                              float t,
                              Ray *bCurrentRay,
                              void(*ColourPalette)(float value, float col[]),
                              bool xyz_is_1[],
                              const lb::StressTypes iLbmStressType);

        void rtTraverseBlocksFn(float ray_dx[],
                                float **block_flow_field,
                                Ray *bCurrentRay,
                                void(*ColourPalette)(float value, float col[]),
                                bool xyz_Is_1[],
                                const lb::StressTypes iLbmStressType);

        void rtAABBvsRayFn(const AABB* aabb,
                           const float inverseDirection[3],
                           const bool xyzComponentIsPositive[3],
                           float* t_near,
                           float* t_far);

        void UpdateColour(float dt, const float palette[3], float col[3]);

        void rtBuildClusters();

        const topology::NetworkTopology * mNetworkTopology;
        const geometry::LatticeData* mLatDat;

        DomainStats* mDomainStats;
        Screen* mScreen;
        Viewpoint* mViewpoint;
        VisSettings* mVisSettings;

        std::vector<Cluster*> mClusters;
        float **cluster_voxel;
        float ***cluster_flow_field;

        int cluster_blocks_vec[3];
        int cluster_blocks_z, cluster_blocks_yz, cluster_blocks;

        const float mBlockSizeFloat;
        const float mBlockSizeInverse;
        const unsigned int block_size2, block_size3, block_size_1;
        const unsigned int blocks_yz;
    };

  }
}

#endif // HEMELB_VIS_RAYTRACER_H
