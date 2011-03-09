#ifndef HEMELB_VIS_COLPIXEL_H
#define HEMELB_VIS_COLPIXEL_H

#include "mpiInclude.h"
#include "vis/DomainStats.h"
#include "vis/ColourPalette.h"
#include "lb/LbmParameters.h"

namespace hemelb
{
  namespace vis
  {
    // TODO: This should really be a temporary header file that grows to have more common stuff in it.

    struct PixelId
    {
        unsigned int isRt :1;
        unsigned int isGlyph :1;
        unsigned int isStreakline :1;
        unsigned int i :14;
        unsigned int j :14;

        PixelId();
        PixelId(int i, int j);
    };

    class ColPixel
    {
      public:
        float vel_r, vel_g, vel_b;
        float stress_r, stress_g, stress_b;
        float t, dt;
        float density;
        float stress;

        float particle_vel;
        float particle_z;

        int particle_inlet_id;

        struct PixelId i;

        /**
         * Merge data from the first ColPixel argument into the second
         * ColPixel argument.
         */
        void MergeIn(const ColPixel *fromPixel, lb::StressTypes iStressType, int mode);

        void rawWritePixel(int *pixel_index,
                           int mode,
                           unsigned char rgb_data[],
                           DomainStats* iDomainStats,
                           ColourPaletteFunction *colourPalette,
                           lb::StressTypes iLbmStressType);
        static const MPI_Datatype& getMpiType();

      protected:
        static void registerMpiType();
        static MPI_Datatype mpiType;

        void makePixelColour(unsigned char& red,
                             unsigned char& green,
                             unsigned char& blue,
                             int rawRed,
                             int rawGreen,
                             int rawBlue);
    };

  }
}

#endif // HEMELB_VIS_COLPIXEL_H
