/*=========================================================================

  Program:   Regrid Volume
  Module:    $RCSfile: regridvolume.h,v $

  Copyright (c) Sean J S Chen
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
 Small modification by Anka
=========================================================================*/
// .NAME regridvolume - reconstructs a 3D volume out of 2D inputs
// Regrids the an image/volume of arbitrary
//  orientation and position in a volume

#ifndef REGRID_VOLUME_H_
#define REGRID_VOLUME_H_

#include "vtkObject.h"
#include <vector>

class vtkImageData;
class ImageObject;
class QString;
class vtkMatrix4x4;
class vtkTransform;
class USAcquisitionInfo;

class RegridVolume : public vtkObject
{
public:
    
    static RegridVolume* New() { return new RegridVolume; }
    virtual void Delete(){}

    RegridVolume();
    virtual ~RegridVolume();

    bool RegridFile(QString& p_rTheString);

    bool RegridImage(vtkImageData* p_pTheImage, vtkMatrix4x4* p_pTheTransform);
 
    void InputFile(QString& p_rTheString, int p_counter = 0, USAcquisitionInfo *p_info = 0); 

    void InputImage(vtkImageData* p_pTheImage, vtkMatrix4x4* p_pTheTransform);

    void SetImageMask(QString& p_rTheString);

    void SetImageMask(vtkImageData* p_pTheMask);

    void SetValueThreshold(double p_theThreshold);

    void SetSaturationBasedExtraction(bool p_doExtract);

    void SetInterpolationType(int p_interpolationType);
    
    void SetInterpolationRadius(double p_interpolationRadius);
    
    void SetStepSize(double p_stepSize);

    void SetSigma(double p_sigma);

    void ClearMask();

    void ClearMe();

    // Description:
    // (Anka) Set modifyName to false if you do not want the regridder to modify the name entered ny the user
    bool GenerateVolume(ImageObject& r_OutputFile, bool modifyName) const;
    bool GenerateVolume(vtkImageData* p_pOutputFile) const;

    static bool IsWorthInterpolation(double myRadius, double myStepSize);

    static int GAUSSIAN_INTERPOLATION;
    static int KAISERBESSEL_INTERPOLATION;
    static int LINEAR_INTERPOLATION;

protected:

    void t_regridWithInterpolation(float* l_pTheVolume, float* l_pTheWeights,
				   vtkImageData* l_pSourceVolumeData, vtkMatrix4x4* l_pDirCos,
				   int l_radiusX, int l_radiusY, int l_radiusZ,
				   double* l_pWeightsLookuptable) const;

    void t_regridItAll(float* l_pTheVolume, float* l_pTheWeights,
		       vtkImageData* l_pSourceVolumeData, vtkMatrix4x4* l_pDirCos) const;

    void t_writeOutResults(float* theVolumeData,
			   float* theWeights,
			   int quantityOfData,
			   int quantityOfTuples,
			   vtkImageData* outputVolume) const;

    void t_generateWeightsLookupTable(int radiusX, int radiusY, int radiusZ,
                double* theLookupTable) const;

    void t_transformVoxelToWorld(double voxelX, double voxelY, double voxelZ,
                const double step[3],
                const double origin[3],
                vtkMatrix4x4* rotation,
                double output_world[4]) const;

    int t_getVolumeVoxelOffset(double x, double y, double z) const;
    
    double t_getVolumeCoordX(double x) const;
    double t_getVolumeCoordY(double y) const;
    double t_getVolumeCoordZ(double z) const;

    double t_squareNum(double num) const;
    double t_cubeNum(double num) const;
    double t_kaiserBesselI0(double value) const;
    float  t_max(float vals[3]) const;
    double t_saturation(double vals[3]) const;

    //output volume bounding box
    double mt_maxX;
    double mt_maxY;
    double mt_maxZ;

    double mt_minX;
    double mt_minY;
    double mt_minZ;

    //output volume voxel stuff
    int mt_length;
    int mt_width;
    int mt_height;

    int mt_volOffsetConstX;
    int mt_volOffsetConstXY;

    int mt_voxelScalarComponents;

    double mt_scalingX; //scaling from world to outputVol Xcoord space
    double mt_scalingY; //scaling from world to outputVol Ycoord space
    double mt_scalingZ; //scaling from world to outputVol Zcoord space

    //interpolating stuff 
    double mt_interpolateType;
    double mt_interpolateRadius;
    double mt_interpolateSigma;
    
    bool mt_bSaturationFilter;
    double mt_minimalSaturation;
    double mt_vesselValueThreshold;

    vtkImageData* mt_pTheMask;
    float* mt_pTheVolume;
    float* mt_pTheWeights;
    double* mt_pWeightsLookuptable;

    int mt_radiusX;
    int mt_radiusY;
    int mt_radiusZ;

    int mt_scalarType;
    bool mt_worthTheWork;
    bool mt_AllowAccess;
};
    
#endif //REGRID_VOLUME_H_
