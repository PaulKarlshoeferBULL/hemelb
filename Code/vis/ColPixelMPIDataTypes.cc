#include "mpiInclude.h"
#include "vis/ColPixel.h"
#include "vis/rayTracer/RayDataNormal.h"
#include "vis/rayTracer/RayDataEnhanced.h"

namespace hemelb
{

  template<>
  MPI_Datatype MpiDataTypeTraits<hemelb::vis::ColPixel<hemelb::vis::raytracer::RayDataNormal> >::RegisterMpiDataType()
  {
    int col_pixel_count = 7;
    int col_pixel_blocklengths[7] = { 1, 1, 1, 1, 1, 1, 1 };

    MPI_Datatype col_pixel_types[7] =
        { MPI_UNSIGNED,
          MPI_UNSIGNED,
          MpiDataTypeTraits<hemelb::vis::raytracer::RayDataNormal>::GetMpiDataType(),
          MPI_FLOAT,
          MPI_FLOAT,
          MPI_INT,
          MPI_UB };

    MPI_Aint col_pixel_disps[7];

    col_pixel_disps[0] = 0;

    for (int i = 1; i < col_pixel_count; i++)
    {
      if (col_pixel_types[i - 1] == MPI_FLOAT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(float) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_INT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1] + (sizeof(int) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_UNSIGNED)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(unsigned) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1]
          == MpiDataTypeTraits<hemelb::vis::raytracer::RayDataNormal>::GetMpiDataType())
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(hemelb::vis::raytracer::RayDataNormal) * col_pixel_blocklengths[i - 1]);
      }

    }
    MPI_Datatype type;
    MPI_Type_struct(col_pixel_count,
                    col_pixel_blocklengths,
                    col_pixel_disps,
                    col_pixel_types,
                    &type);
    MPI_Type_commit(&type);
    return type;
  }

  template<>
  MPI_Datatype MpiDataTypeTraits<
      hemelb::vis::ColPixel<
          hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::MIST> > >::RegisterMpiDataType()
  {
    int col_pixel_count = 7;
    int col_pixel_blocklengths[7] = { 1, 1, 1, 1, 1, 1, 1 };

    MPI_Datatype col_pixel_types[7] =
        { MPI_UNSIGNED,
          MPI_UNSIGNED,
          MpiDataTypeTraits<
              hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::MIST> >::GetMpiDataType(),
          MPI_FLOAT,
          MPI_FLOAT,
          MPI_INT,
          MPI_UB };

    MPI_Aint col_pixel_disps[7];

    col_pixel_disps[0] = 0;

    for (int i = 1; i < col_pixel_count; i++)
    {
      if (col_pixel_types[i - 1] == MPI_FLOAT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(float) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_INT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1] + (sizeof(int) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_UNSIGNED)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(unsigned) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1]
          == MpiDataTypeTraits<
              hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::MIST> >::GetMpiDataType())
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::MIST>)
                * col_pixel_blocklengths[i - 1]);
      }

    }
    MPI_Datatype type;
    MPI_Type_struct(col_pixel_count,
                    col_pixel_blocklengths,
                    col_pixel_disps,
                    col_pixel_types,
                    &type);
    MPI_Type_commit(&type);
    return type;
  }

  template<>
  MPI_Datatype MpiDataTypeTraits<
      hemelb::vis::ColPixel<
          hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::DARKNESS> > >::RegisterMpiDataType()
  {
    int col_pixel_count = 7;
    int col_pixel_blocklengths[7] = { 1, 1, 1, 1, 1, 1, 1 };

    MPI_Datatype col_pixel_types[7] =
        { MPI_UNSIGNED,
          MPI_UNSIGNED,
          MpiDataTypeTraits<
              hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::DARKNESS> >::GetMpiDataType(),
          MPI_FLOAT,
          MPI_FLOAT,
          MPI_INT,
          MPI_UB };

    MPI_Aint col_pixel_disps[7];

    col_pixel_disps[0] = 0;

    for (int i = 1; i < col_pixel_count; i++)
    {
      if (col_pixel_types[i - 1] == MPI_FLOAT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(float) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_INT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1] + (sizeof(int) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_UNSIGNED)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(unsigned) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1]
          == MpiDataTypeTraits<
              hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::DARKNESS> >::GetMpiDataType())
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::DARKNESS>)
                * col_pixel_blocklengths[i - 1]);
      }

    }
    MPI_Datatype type;
    MPI_Type_struct(col_pixel_count,
                    col_pixel_blocklengths,
                    col_pixel_disps,
                    col_pixel_types,
                    &type);
    MPI_Type_commit(&type);
    return type;
  }

  template<>
  MPI_Datatype MpiDataTypeTraits<
      hemelb::vis::ColPixel<
          hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::NONE> > >::RegisterMpiDataType()
  {
    int col_pixel_count = 7;
    int col_pixel_blocklengths[7] = { 1, 1, 1, 1, 1, 1, 1 };

    MPI_Datatype col_pixel_types[7] =
        { MPI_UNSIGNED,
          MPI_UNSIGNED,
          MpiDataTypeTraits<
              hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::NONE> >::GetMpiDataType(),
          MPI_FLOAT,
          MPI_FLOAT,
          MPI_INT,
          MPI_UB };

    MPI_Aint col_pixel_disps[7];

    col_pixel_disps[0] = 0;

    for (int i = 1; i < col_pixel_count; i++)
    {
      if (col_pixel_types[i - 1] == MPI_FLOAT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(float) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_INT)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1] + (sizeof(int) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1] == MPI_UNSIGNED)
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(unsigned) * col_pixel_blocklengths[i - 1]);
      }
      else if (col_pixel_types[i - 1]
          == MpiDataTypeTraits<
              hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::NONE> >::GetMpiDataType())
      {
        col_pixel_disps[i] = col_pixel_disps[i - 1]
            + (sizeof(hemelb::vis::raytracer::RayDataEnhanced<vis::raytracer::DepthCuing::NONE>)
                * col_pixel_blocklengths[i - 1]);
      }

    }
    MPI_Datatype type;
    MPI_Type_struct(col_pixel_count,
                    col_pixel_blocklengths,
                    col_pixel_disps,
                    col_pixel_types,
                    &type);
    MPI_Type_commit(&type);
    return type;
  }
}
