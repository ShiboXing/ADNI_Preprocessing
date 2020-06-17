/*=========================================================================

Program:   Regrid Volume
Module:    $RCSfile: regridvolume.cpp,v $

Copyright (c) Sean J S Chen
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

 Small modification by Anka
=========================================================================*/
// .NAME regridvolume - reconstructs a 3D volume out of 2D inputs
//A complete reimplementation and integration of volregrid-like functions into IBIS
//Completed Sept 3 2007, with minor hacks and add-ons following after
//Redone to allow "streaming" without buffering on Oct 25 at cost of usability

#include <iostream>
#include <cmath>

#include <qmessagebox.h>
#include <qstring.h>
#include <qprogressdialog.h>

#include "vtkObject.h"
#include "vtkReal.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

#include "ignsmsg.h"
#include "imageobject.h"
#include "usacquisitioninfo.h"
#include "vtkMINCImageAttributes2.h"
#include "attrib2usacquisitioninfo.h"

#include "MINCReader.h"

#include "regridvolume.h"

int RegridVolume::GAUSSIAN_INTERPOLATION = 0;
int RegridVolume::KAISERBESSEL_INTERPOLATION = 1;
int RegridVolume::LINEAR_INTERPOLATION = 2;

RegridVolume::RegridVolume():
    mt_maxX(VTK_INT_MIN),
    mt_maxY(VTK_INT_MIN),
    mt_maxZ(VTK_INT_MIN),
    mt_minX(VTK_INT_MAX),
    mt_minY(VTK_INT_MAX),
    mt_minZ(VTK_INT_MAX),
    mt_length(0),
    mt_width(0),
    mt_height(0),
    mt_volOffsetConstX(0),
    mt_volOffsetConstXY(0),
    mt_voxelScalarComponents(1),
    mt_scalingX(0.3),
    mt_scalingY(0.3),
    mt_scalingZ(0.3),
    mt_interpolateType(GAUSSIAN_INTERPOLATION),
    mt_interpolateRadius(0.5),
    mt_interpolateSigma(mt_interpolateRadius * 1.2),
    mt_bSaturationFilter(0),
    mt_minimalSaturation(0.205),
    mt_vesselValueThreshold(0.85),
    mt_pTheMask(0),
    mt_pTheVolume(0),
    mt_pTheWeights(0),
    mt_pWeightsLookuptable(0),
    mt_radiusX(0),
    mt_radiusY(0),
    mt_radiusZ(0),
    mt_scalarType(0),
    mt_worthTheWork(0),
    mt_AllowAccess(true) //This basically toggles between bBox calc and regrid mode
{
}

RegridVolume::~RegridVolume()
{
    ClearMe();
    ClearMask();
}


void RegridVolume::InputFile(QString& p_rTheString, int p_counter, USAcquisitionInfo *p_info)
{
    if(mt_AllowAccess)
    {
        MINCReader* l_pMincReader = MINCReader::New();
        l_pMincReader->SetFileName(p_rTheString.ascii());
        l_pMincReader->Update();
	if (l_pMincReader->GetErrorCode())
	{
	    QMessageBox::warning( 0, "Warning", IGNS_MSG_MINC_CORRUPTED, 1, 0 );
	}
	else
	{
            if ( p_counter == 0 &&  p_info != 0)
            {
                vtkMINCImageAttributes2 *attributes = l_pMincReader->GetImageAttributes();
                Attrib2USAcquisitionInfo *tmpInfo = new Attrib2USAcquisitionInfo;
                tmpInfo->SetUSAcquisitionInfo(p_info);
                tmpInfo->FillUSAcquisitionInfo(attributes);
                delete tmpInfo;
            }     
            InputImage(l_pMincReader->GetOutput(), l_pMincReader->GetTransformationMatrix());
	}

	//delete the minc reader
	l_pMincReader->Delete();
    }
}

void RegridVolume::InputImage(vtkImageData* p_pTheImage, vtkMatrix4x4* p_pTheTransform)
{

    if(mt_AllowAccess)
    {
	if(p_pTheImage->GetDimensions()[2] != 1)
	{
	    QMessageBox::warning(0,"Error", "Error: We currently only support the regridding of 2D minc images. Sorry!", 1, 0);
	}
	else
	{
	    //This if structure makes sure the subsequent images are of the same
	    //scalar components
	    if(mt_maxX == VTK_INT_MIN)
	    {
		mt_voxelScalarComponents = p_pTheImage->GetNumberOfScalarComponents();
	    }
	    else
	    {
		if(mt_voxelScalarComponents != p_pTheImage->GetNumberOfScalarComponents())
		    return;
	    }

	    int* l_aDimensions = p_pTheImage->GetDimensions();
	    const double* l_pOrigin = p_pTheImage->GetOrigin();
	    const double* l_pStep = p_pTheImage->GetSpacing();

	    //Go through the eight corners of the source volume and find the
	    //fitting bounding box
	    double l_aResult[4];
	    for(int i=0; i < 2; ++i)
	    {
		for(int j=0; j < 2; ++j)
		{
		    for(int k=0; k < 2; ++k)
		    {
			t_transformVoxelToWorld(i * l_aDimensions[0],
						j * l_aDimensions[1],
						k * l_aDimensions[2], 
						l_pStep, l_pOrigin, p_pTheTransform, l_aResult);
            
			if(l_aResult[0] > mt_maxX) mt_maxX = l_aResult[0];
			if(l_aResult[0] < mt_minX) mt_minX = l_aResult[0];
			if(l_aResult[1] > mt_maxY) mt_maxY = l_aResult[1];
			if(l_aResult[1] < mt_minY) mt_minY = l_aResult[1];
			if(l_aResult[2] > mt_maxZ) mt_maxZ = l_aResult[2];
			if(l_aResult[2] < mt_minZ) mt_minZ = l_aResult[2];
		    }
		}
	    }

	    mt_length = (int)ceil((mt_maxX - mt_minX) / mt_scalingX);
	    mt_width = (int)ceil((mt_maxY - mt_minY) / mt_scalingY);
	    mt_height  = (int)ceil((mt_maxZ - mt_minZ) / mt_scalingZ);
    
	    //calculate the looping offsets for a volume of this size
	    mt_volOffsetConstX = mt_length * mt_voxelScalarComponents;
	    mt_volOffsetConstXY = mt_length * mt_width * mt_voxelScalarComponents;

	    mt_radiusX = (int)floor(mt_interpolateRadius/mt_scalingX);
	    mt_radiusY = (int)floor(mt_interpolateRadius/mt_scalingY);
	    mt_radiusZ = (int)floor(mt_interpolateRadius/mt_scalingZ);
	}
    }
}

void RegridVolume::SetImageMask(QString& p_rTheString)
{
    if(mt_AllowAccess)
    {
        MINCReader* l_pMincReader = MINCReader::New();
        l_pMincReader->SetFileName(p_rTheString.ascii());
        l_pMincReader->Update();

	if (l_pMincReader->GetErrorCode())
	{
	    QMessageBox::warning( 0, "Warning", IGNS_MSG_MINC_CORRUPTED, 1, 0 );
        }
        else
        {   
            SetImageMask(l_pMincReader->GetOutput());
        }

        //delete the mince reader
        l_pMincReader->Delete();
    }
}

void RegridVolume::SetImageMask(vtkImageData* p_pTheMask)
{
    if(mt_AllowAccess)
    {
	if(p_pTheMask == 0)
	{
	    QMessageBox::warning( 0, "Warning", "WARNING:Mask does not exist, not setting mask", 1, 0);
	}
	else
	{
	    if(mt_pTheMask == 0)
	    {
		mt_pTheMask = vtkImageData::New();
	    }
	    mt_pTheMask->DeepCopy(p_pTheMask);
	}
    }
}

void RegridVolume::SetInterpolationType(int p_interpolationType)
{
    if(mt_AllowAccess)
    {
	mt_interpolateType = p_interpolationType;
    }
}
    
void RegridVolume::SetInterpolationRadius(double p_interpolationRadius)
{
    if(mt_AllowAccess)
    {
	mt_interpolateRadius = p_interpolationRadius;
	mt_interpolateSigma = mt_interpolateRadius * 2;
    
	mt_radiusX = (int)floor(mt_interpolateRadius/mt_scalingX);
	mt_radiusY = (int)floor(mt_interpolateRadius/mt_scalingY);
	mt_radiusZ = (int)floor(mt_interpolateRadius/mt_scalingZ);
    }
}

void RegridVolume::SetStepSize(double p_stepSize)
{
    if(mt_AllowAccess)
    {
	//Recompute the axis scalings
	mt_scalingX = p_stepSize; 
	mt_scalingY = p_stepSize; 
	mt_scalingZ = p_stepSize; 

	mt_length = (int)ceil((mt_maxX - mt_minX) / mt_scalingX);
	mt_width = (int)ceil((mt_maxY - mt_minY) / mt_scalingY);
	mt_height  = (int)ceil((mt_maxZ - mt_minZ) / mt_scalingZ);
    
	mt_volOffsetConstX = mt_length * mt_voxelScalarComponents;
	mt_volOffsetConstXY = mt_length * mt_width * mt_voxelScalarComponents;
    }
}

inline bool RegridVolume::IsWorthInterpolation(double myRadius, double myStepSize)
{
    //if the radius in volume space is more than the length of
    //half a voxel then it's worth interpolating, otherwise
    //it's not
    return myRadius/myStepSize > 0.5;
}

void RegridVolume::SetValueThreshold(double p_theThreshold)
{
    if(mt_AllowAccess)
    {
	mt_vesselValueThreshold = p_theThreshold;
    }
}

void RegridVolume::SetSaturationBasedExtraction(bool p_doExtract)
{
    if(mt_AllowAccess)
    {
	mt_bSaturationFilter = p_doExtract;
    }
}

void RegridVolume::SetSigma(double p_sigma)
{
    if(mt_AllowAccess)
    {
	mt_interpolateSigma = p_sigma;
    }
}

bool RegridVolume::RegridFile(QString& p_rTheString)
{
    MINCReader* l_pMincReader = MINCReader::New();
    l_pMincReader->SetFileName(p_rTheString.ascii());
    l_pMincReader->Update();

    bool retVal = 0;
    
    if (l_pMincReader->GetErrorCode())
    {
        QMessageBox::warning( 0, "Warning", IGNS_MSG_MINC_CORRUPTED, 1, 0 );
        return false;
    }
    else
    {   
        retVal = RegridImage(l_pMincReader->GetOutput(), l_pMincReader->GetTransformationMatrix());
    }

    //delete the mince reader
    l_pMincReader->Delete();
    return retVal;
}


bool RegridVolume::RegridImage(vtkImageData* p_pTheImage, vtkMatrix4x4* p_pTheTransform)
{

    // if nothing has been initiated then we stop
    if(mt_maxX == VTK_INT_MIN)
    {
	return false;
    }

    if(mt_AllowAccess)
    {
	mt_AllowAccess = false;	
    }

    if(mt_pTheVolume == 0)
    {
	
	//Determine the amount of space needed for the data
	int l_quantityOfData = mt_length * mt_width * mt_height;
	l_quantityOfData *= (mt_bSaturationFilter)? 1 : mt_voxelScalarComponents;

	//Determine the amount space needed for the weights to normalize the data
	int l_quantityOfTuples = mt_length * mt_width * mt_height;
	
	//variable of dummy data
	//This is done for cheating and making sure that the system will have
	//enough memory to allocate for the output object (ImageObject& p_rOutputFile at
	//Generate Volume) where data will be copied over from mt_pTheVolume and 
	//mt_pTheWeights after processing. We assume that the memory available will not
	//change in the course of the program's running...or else...segfault
	double* dummy;

	//Allocate the space in arrays. You might ask why I don't put them directly
	//into a vtkImageData object and save some space in the "copying-over"
	//process. The reason is that dealing with vtk-whatevers is good deal
	//slower and a lot less elegant in terms of coding (no stuff like a[] /= b[])
	try
	{
	    //float since this is all the precision we need in terms of image real values
	    mt_pTheVolume = new float[l_quantityOfData]; 
	    mt_pTheWeights = new float[l_quantityOfTuples];
	    
	    dummy = new double[l_quantityOfData];
	}
	catch(std::bad_alloc)
	{
	    QMessageBox::warning( 0, "Error",
				  "The memory on your machine cannot fit the reconstructed volume.\n\nIncrease the step size, or better yet, GO BUY MORE MEMORY", 1, 0 );

	    if(mt_pTheVolume != 0) delete mt_pTheVolume;
	    if(mt_pTheWeights != 0) delete mt_pTheWeights;
	    
	    mt_pTheVolume = 0;
	    mt_pTheWeights = 0;
	    
	    return false;
	}
	///data/ipl/proj01/monkeys/paxinos/louis$

	//delete dummy data
	delete dummy;

	//if we are not interpolating across more than 1 voxel, than don't interpolate
	mt_worthTheWork = IsWorthInterpolation(mt_interpolateRadius, mt_scalingX) &&
	    IsWorthInterpolation(mt_interpolateRadius, mt_scalingY) &&
	    IsWorthInterpolation(mt_interpolateRadius, mt_scalingZ);
	
	if(mt_worthTheWork)
	{
	    //If we are interpolating then generate the lookup table to speedup the
	    //weights calculation
	    mt_pWeightsLookuptable = new double[(mt_radiusX+1) * (mt_radiusY+1) * (mt_radiusZ+1)];
	    t_generateWeightsLookupTable(mt_radiusX, mt_radiusY, mt_radiusZ, mt_pWeightsLookuptable);
	}

	mt_scalarType = p_pTheImage->GetScalarType();    
    }

    if(p_pTheImage->GetDimensions()[2] != 1)
    {
	QMessageBox::warning(0,"Error", "Error: We currently only support the regridding of 2D minc images. Sorry!", 1, 0);
    }
    else
    {
	//Make sure the dimensions of the mask are the same as the image being processed
	//That is, if a mask exists
	if(mt_pTheMask != 0 && (mt_pTheMask->GetDimensions()[0] != p_pTheImage->GetDimensions()[0] || 
				mt_pTheMask->GetDimensions()[1] != p_pTheImage->GetDimensions()[1] ||
				mt_pTheMask->GetDimensions()[2] != p_pTheImage->GetDimensions()[2]))
	{
	    QMessageBox::warning( 0, "Error", "Mask and Images DIMENSIONS are different!!!! \nI don't allow.", 1, 0 );
	    
	    if(mt_pTheVolume != 0) delete mt_pTheVolume;
	    if(mt_pTheWeights != 0) delete mt_pTheWeights;
	    return false;
	}

	//Call the regridding functions with respect to interpolation requirements
	if(mt_worthTheWork)
	{
	    t_regridWithInterpolation(mt_pTheVolume, mt_pTheWeights,
				      p_pTheImage, p_pTheTransform,
				      mt_radiusX, mt_radiusY, mt_radiusZ,
				      mt_pWeightsLookuptable);
	}
	else
	{
	    t_regridItAll(mt_pTheVolume, mt_pTheWeights,
			  p_pTheImage,
			  p_pTheTransform);
	}
    }
    return true;
}

// Anka added control for name modification
bool RegridVolume::GenerateVolume(ImageObject& p_rOutputFile, bool modifyName) const
{
    if(!mt_AllowAccess && mt_pTheVolume != 0 && mt_pTheWeights != 0)
    {
	int l_quantityOfData = mt_length * mt_width * mt_height;
	l_quantityOfData *= (mt_bSaturationFilter)? 1 : mt_voxelScalarComponents;

	int l_quantityOfTuples = mt_length * mt_width * mt_height;
    
	//Setup the new output volume
	vtkImageData* l_pOutputVolume = vtkImageData::New();
	
	//Write out results with additional processing if needed
	t_writeOutResults(mt_pTheVolume,
			  mt_pTheWeights,
			  l_quantityOfData,
			  l_quantityOfTuples,
			  l_pOutputVolume);

	//set the data into the output image
	p_rOutputFile.SetImage(l_pOutputVolume);


        if (modifyName)
        {
	//Modifies the name. Don't know if this considered "good programming"
	//since the user of this class may not know that calling this procedure
	//does this modification to their inputted object
            p_rOutputFile.SetName(p_rOutputFile.GetName()+ "_" + QString::number(mt_interpolateRadius) + "rad"
			      + "_" + QString::number((mt_scalingX + mt_scalingY + mt_scalingZ)/3,'g',3) + "step"
			      + "sliceVol");
        }
	//remove the image
	l_pOutputVolume->Delete();

	return true;
    }
    else return false;
}

bool RegridVolume::GenerateVolume(vtkImageData* p_pOutputFile) const
{
    if(!mt_AllowAccess && mt_pTheVolume != 0 && mt_pTheWeights != 0)
    {
	int l_quantityOfData = mt_length * mt_width * mt_height;
	l_quantityOfData *= (mt_bSaturationFilter)? 1 : mt_voxelScalarComponents;

	int l_quantityOfTuples = mt_length * mt_width * mt_height;
    
	//Write out results with additional processing if needed
	t_writeOutResults(mt_pTheVolume,
			  mt_pTheWeights,
			  l_quantityOfData,
			  l_quantityOfTuples,
			  p_pOutputFile);

	//Modifies the name. Don't know if this considered "good programming"
	//since the user of this class may not know that calling this procedure
	//does this modification to their inputted object
// 	p_rOutputFile.SetName(p_rOutputFile.GetName()+ "_" + QString::number(mt_interpolateRadius) + "rad"
// 			      + "_" + QString::number((mt_scalingX + mt_scalingY + mt_scalingZ)/3,'g',3) + "step"
// 			      + "sliceVol");

	return true;
    }
    else return false;
}



void RegridVolume::ClearMask()
{
    if(mt_AllowAccess)
    {
	if(mt_pTheMask != 0)
	{
	    mt_pTheMask->Delete();
	}
	mt_pTheMask = 0;
    }
}


void RegridVolume::ClearMe()
{
    if(mt_pTheVolume != 0) delete[] mt_pTheVolume;
    if(mt_pTheWeights != 0) delete[] mt_pTheWeights;
    if(mt_pWeightsLookuptable !=0) delete[] mt_pWeightsLookuptable;

    mt_pWeightsLookuptable = 0;
    mt_pTheVolume = 0;
    mt_pTheWeights = 0;

    //Important to reset these since they tell this program
    //if images have been inputted
    mt_maxX = VTK_INT_MIN;
    mt_maxY = VTK_INT_MIN;
    mt_maxZ = VTK_INT_MIN;
    mt_minX = VTK_INT_MAX;
    mt_minY = VTK_INT_MAX;
    mt_minZ = VTK_INT_MAX;

    mt_AllowAccess = true;
}


void RegridVolume::t_regridWithInterpolation(float* l_pTheVolume, float* l_pTheWeights,
					     vtkImageData* l_pSourceVolumeData, vtkMatrix4x4* l_pDirCos,
					     int l_radiusX, int l_radiusY, int l_radiusZ,
					     double* l_pWeightsLookuptable) const
{
    //********* START: Stuff for use inside the for loops*********

    //Stores the loop incrementers
    int sideI, sideJ, sideK;
    
    //Stores the temporary transformed locations
    double l_aTempPoint[4];
    
    //Stores the source voxel Data
    double* l_pVoxelData;

    //********* END: Stuff for use inside the for loops********

    ASSERT(l_pSourceVolumeData->GetNumberOfScalarComponents() == mt_voxelScalarComponents);

    //Record data into pointer variable so you don't have to keep jumping
    //from one pointer to another
    double* l_pSteps = l_pSourceVolumeData->GetSpacing();
    double* l_pOrigin = l_pSourceVolumeData->GetOrigin();
    int* l_pDimensions = l_pSourceVolumeData->GetDimensions();
    

    vtkDataArray* l_pSourceVolumeScalars = l_pSourceVolumeData->GetPointData()->GetScalars();

    int l_currentMaxI = l_pDimensions[0];
    int l_currentMaxJ = l_pDimensions[1];
    int l_currentMaxK = l_pDimensions[2];

//    int l_offsetI=0;
    int l_offsetJ=0;
    int l_offsetK=0;

    int l_offsetConstI = l_currentMaxI;
    int l_offsetConstIJ = l_offsetConstI * l_currentMaxJ;
    


    //********* START: Stuff for inner loops*********
    int      i, j, k;
    int      offset;
    float   weight;
    int l_radiusZY;

    int startPosX;
    int centrePosX;
    int stopPosX;
   
    int startPosY;
    int centrePosY;
    int stopPosY;
   
    int startPosZ;
    int centrePosZ;
    int stopPosZ;

    int l_weightsOffsetZYRadiuscentrePosXY;
    int l_weightsOffsetZYRadiuscentrePosX;
    
    int l_neighborhoodOffsetXY;
    int l_neighborhoodOffsetX;
    //********* END: Stuff for inner loops********

    //Loop through all the voxels in the image and transform them to the world coordinates
    for(sideK=0; sideK<l_currentMaxK; ++sideK)
    {
	l_offsetK= sideK * l_offsetConstIJ;
    
	for(sideJ=0; sideJ<l_currentMaxJ; ++sideJ)
	{
	    l_offsetJ= l_offsetK + (sideJ * l_offsetConstI);
    
	    for(sideI=0; sideI<l_currentMaxI; ++sideI)
	    {
		l_pVoxelData = l_pSourceVolumeScalars->GetTuple(l_offsetJ + sideI);

		if(!mt_pTheMask || *(mt_pTheMask->GetPointData()->GetScalars()->GetTuple(l_offsetJ + sideI)) != 0.0)
		{
		    //transform the 2D image voxel to their world coordinates
		    t_transformVoxelToWorld(sideI, sideJ, sideK,
					    l_pSteps, 
					    l_pOrigin,
					    l_pDirCos,
					    l_aTempPoint);
		    
		    l_radiusZY = l_radiusZ*l_radiusY;

		    //get the neighboring voxels based on the double world position of the world point
		    startPosX = (int)floor(t_getVolumeCoordX(l_aTempPoint[0] - mt_interpolateRadius));
		    centrePosX = (int)rint(t_getVolumeCoordX(l_aTempPoint[0]));
		    stopPosX = (int)floor(t_getVolumeCoordX(l_aTempPoint[0] + mt_interpolateRadius));
		    if(startPosX < 0) startPosX = 0;
		    if(stopPosX >= mt_length) stopPosX = mt_length - 1;

		    startPosY = (int)floor(t_getVolumeCoordY(l_aTempPoint[1] - mt_interpolateRadius));
		    centrePosY = (int)rint(t_getVolumeCoordY(l_aTempPoint[1]));
		    stopPosY = (int)floor(t_getVolumeCoordY(l_aTempPoint[1] + mt_interpolateRadius));
		    if(startPosY < 0) startPosY = 0;
		    if(stopPosY >= mt_width) stopPosY = mt_width - 1;

		    startPosZ = (int)floor(t_getVolumeCoordZ(l_aTempPoint[2] - mt_interpolateRadius));
		    centrePosZ = (int)rint(t_getVolumeCoordZ(l_aTempPoint[2]));
		    stopPosZ = (int)floor(t_getVolumeCoordZ(l_aTempPoint[2] + mt_interpolateRadius));
		    if(startPosZ < 0) startPosZ = 0;
		    if(stopPosZ >= mt_height) stopPosZ = mt_height - 1;

		    for(k = startPosZ; k <= stopPosZ; ++k)
		    {
			l_weightsOffsetZYRadiuscentrePosXY = l_radiusZY*abs(k - centrePosZ);
			l_neighborhoodOffsetXY = (k * mt_volOffsetConstXY);
			for(j = startPosY; j <= stopPosY; ++j)
			{
			    l_weightsOffsetZYRadiuscentrePosX = l_radiusZ*abs(j - centrePosY) + l_weightsOffsetZYRadiuscentrePosXY;
			    l_neighborhoodOffsetX = (j *  mt_volOffsetConstX) + l_neighborhoodOffsetXY;

			    for(i = startPosX; i <= stopPosX; ++i)
			    {
				//get the weight based the relative neightbor position
				weight = (float)l_pWeightsLookuptable[l_weightsOffsetZYRadiuscentrePosX
								      + abs(i - centrePosX)];
    
				//calculate this position's offset value for the output arrays
				offset = l_neighborhoodOffsetX + (i * mt_voxelScalarComponents);
    
          
				if(mt_bSaturationFilter)
				{
				    if(t_saturation(l_pVoxelData) > mt_minimalSaturation)
				    {
					l_pTheVolume[offset/mt_voxelScalarComponents] += (l_pVoxelData[0] + l_pVoxelData[1] + l_pVoxelData[2])/3.0 * weight;
					
					l_pTheWeights[offset/mt_voxelScalarComponents]+= weight;
				    }
				}
				else
				{
				    l_pTheWeights[offset/mt_voxelScalarComponents] += weight; //note that for the weights we ignore the mt_voxelScalarComponents
				    l_pTheVolume[offset] += (float)(l_pVoxelData[0] * weight);
    
				    //if RGB or what-not, then do more stuff
				    if(!mt_bSaturationFilter && mt_voxelScalarComponents == 3) 
				    {
					l_pTheVolume[offset+1] += (float)(l_pVoxelData[1] * weight);
					l_pTheVolume[offset+2] += (float)(l_pVoxelData[2] * weight);
				    }
				}
			    }
			}
		    }

		}
	    }
	}
    }
}


void RegridVolume::t_regridItAll(float* l_pTheVolume, float* l_pTheWeights,
				 vtkImageData* l_pSourceVolumeData, vtkMatrix4x4* l_pDirCos) const
					   
{
    //********* START: Stuff for use inside the for loops*********

    //Stores the loop incrementers
    int sideI, sideJ, sideK;
    
    //Stores the temporary transformed locations
    double l_aTempPoint[4];
    
    //Stores the source voxel Data
    double* l_pVoxelData;

    //Stores the offset of the source Data
    int l_offset;

    //********* END: Stuff for use inside the for loops********

    ASSERT(l_pSourceVolumeData->GetNumberOfScalarComponents() == mt_voxelScalarComponents);

    //Record data into pointer variable so you don't have to keep jumping
    //from one pointer to another
    double* l_pSteps = l_pSourceVolumeData->GetSpacing();
    double* l_pOrigin = l_pSourceVolumeData->GetOrigin();
    int* l_pDimensions = l_pSourceVolumeData->GetDimensions();
    

    vtkDataArray* l_pSourceVolumeScalars = l_pSourceVolumeData->GetPointData()->GetScalars();

    int l_currentMaxI = l_pDimensions[0];
    int l_currentMaxJ = l_pDimensions[1];
    int l_currentMaxK = l_pDimensions[2];

//    int l_offsetI=0;
    int l_offsetJ=0;
    int l_offsetK=0;

    int l_offsetConstI = l_currentMaxI;
    int l_offsetConstIJ = l_offsetConstI * l_currentMaxJ;

    //Loop through all the voxels in the image and transform them to the world coordinates
    for(sideK=0; sideK<l_currentMaxK; ++sideK)
    {
	l_offsetK= sideK * l_offsetConstIJ;
    
	for(sideJ=0; sideJ<l_currentMaxJ; ++sideJ)
	{
	    l_offsetJ= l_offsetK + (sideJ * l_offsetConstI);
    
	    for(sideI=0; sideI<l_currentMaxI; ++sideI)
	    {
		l_pVoxelData = l_pSourceVolumeScalars->GetTuple(l_offsetJ + sideI);

		if(!mt_pTheMask || *(mt_pTheMask->GetPointData()->GetScalars()->GetTuple(l_offsetJ + sideI)) != 0.0)
		{
		    //transform the 2D image voxel to their world coordinates
		    t_transformVoxelToWorld(sideI, sideJ, sideK,
					    l_pSteps, 
					    l_pOrigin,
					    l_pDirCos,
					    l_aTempPoint);
    
		    //else we just regrid
		    l_offset = t_getVolumeVoxelOffset(l_aTempPoint[0], l_aTempPoint[1], l_aTempPoint[2]);
			    
		    if(mt_bSaturationFilter)
		    {
			if(t_saturation(l_pVoxelData) > mt_minimalSaturation)
			{
			    //mt_vesselValueThreshold

			    l_pTheVolume[l_offset/mt_voxelScalarComponents] += (l_pVoxelData[0] + l_pVoxelData[1] + l_pVoxelData[2])/3.0;
			    
			    ++l_pTheWeights[l_offset/mt_voxelScalarComponents];
			}
		    }
		    else
		    {
			++l_pTheWeights[l_offset/ mt_voxelScalarComponents];
			l_pTheVolume[l_offset] += (float)l_pVoxelData[0];
			
			if(mt_voxelScalarComponents == 3) 
			{
			    l_pTheVolume[l_offset+1] += (float)l_pVoxelData[1];
			    l_pTheVolume[l_offset+2] += (float)l_pVoxelData[2];
			}
		    }
		}
	    }
	}
    }
}


void RegridVolume::t_writeOutResults(float* p_pTheVolumeData,
				     float* p_ptheWeights,
				     int p_pQuantityOfData,
				     int p_pQuantityOfTuples,
				     vtkImageData* p_pOutputVolume) const
{
    //Setup the new output volume, with info we should have already
    //gleaned from the image data
    p_pOutputVolume->SetDimensions(mt_length, mt_width, mt_height);
    p_pOutputVolume->SetOrigin(mt_minX, mt_minY, mt_minZ);

    //We use the first image as the guide in order to to determine the
    //correct scalartype
    p_pOutputVolume->SetScalarType(mt_scalarType);

    //The inverse of the volume scaling is the step size of the volume
    p_pOutputVolume->SetSpacing(mt_scalingX, mt_scalingY, mt_scalingZ);

    //This if-structure write the date from the arrays into the output imageobject
    if(mt_voxelScalarComponents == 1)
    {
	/*******
	 *  This is for writing out a plain greyscale file
	 *******/

	//Allocate memory for p_pOutputVolume, 
	p_pOutputVolume->SetNumberOfScalarComponents(mt_voxelScalarComponents);
	p_pOutputVolume->AllocateScalars();

	//The pointer for the volume being written
	vtkDataArray* l_pOutputVolumeScalars = p_pOutputVolume->GetPointData()->GetScalars();

	for(int volIndex =0; volIndex < p_pQuantityOfData; ++volIndex)
	{
	    l_pOutputVolumeScalars->SetTuple1(volIndex,
					      p_pTheVolumeData[volIndex] / p_ptheWeights[volIndex]);
 	}

    }
    else if(mt_voxelScalarComponents == 3)
    {
	if(mt_bSaturationFilter)
	{
	    /*******
	     *  This is for writing out a greyscale file through
	     *  filtering to get only places where there are colour values 
	     *******/

	    //Allocate memory for p_pOutputVolume, 
	    p_pOutputVolume->SetNumberOfScalarComponents(1);
	    p_pOutputVolume->AllocateScalars();

	    //The pointer for the volume being written
	    vtkDataArray* l_pOutputVolumeScalars = p_pOutputVolume->GetPointData()->GetScalars();
	    float tempValue = 0.0;
	    
	    for(int volIndex =0; volIndex < p_pQuantityOfData; ++volIndex)
	    {
		if(isnan(p_pTheVolumeData[volIndex]))	
		    l_pOutputVolumeScalars->SetTuple1(volIndex, -1);
		else
		{
		    tempValue = p_pTheVolumeData[volIndex] / p_ptheWeights[volIndex];
		    l_pOutputVolumeScalars->SetTuple1(volIndex, 
						      (mt_vesselValueThreshold < tempValue)? tempValue:0);
		}
	    }
	}
	else
	{
	    /*******
	     *  This is for writing out a plain "colour" file
	     *******/
	    int tupleIndex = 0;

	    //Allocate memory for p_pOutputVolume, 
// 	    p_pOutputVolume->SetNumberOfScalarComponents(mt_voxelScalarComponents);
	    p_pOutputVolume->SetNumberOfScalarComponents(1);
	    p_pOutputVolume->AllocateScalars();

	    //The pointer for the volume being written
	    vtkDataArray* l_pOutputVolumeScalars = p_pOutputVolume->GetPointData()->GetScalars();
	    float tempValue = 0.0;

	    for(int volIndex =0; volIndex < p_pQuantityOfTuples; ++volIndex)
	    { 

//                 l_pOutputVolumeScalars->SetTuple3(volIndex,
//                                                   p_pTheVolumeData[tupleIndex] / p_ptheWeights[volIndex],
//                                                   p_pTheVolumeData[tupleIndex+1] / p_ptheWeights[volIndex],
//                                                   p_pTheVolumeData[tupleIndex+2] / p_ptheWeights[volIndex]);
		if(isnan(p_pTheVolumeData[tupleIndex]))		    
		    l_pOutputVolumeScalars->SetTuple1(volIndex, -1);
		else
		{
		    //tempValue = t_max(p_pTheVolumeData)/ p_ptheWeights[volIndex];
			
		    tempValue = (p_pTheVolumeData[tupleIndex] + p_pTheVolumeData[tupleIndex+1] + p_pTheVolumeData[tupleIndex+2])/ (3 * p_ptheWeights[volIndex]);
		    l_pOutputVolumeScalars->SetTuple1(volIndex,
						      (mt_vesselValueThreshold < tempValue)? tempValue:0);
		}

		tupleIndex += mt_voxelScalarComponents;
	    }
	}
    }
    else
    {
	std::cerr<<"Scalar components that are not of 1 or 3 are not supported. Blowing chucks"<<std::endl;
    }
}


void RegridVolume::t_generateWeightsLookupTable(int lookupX, int lookupY, int lookupZ,
						double* theLookupTable) const
{
    //Creates a lookup table where the euclidian distances are pre-calculated from
    //that world positions of a set of voxels. The weights are then calculated using
    //these distances by applying them to verious interpolation functions. By removing
    //the need of having to calculate these weights repeatedly on the fly, we get a
    //good deal of interpolation speedup
    double l_euclidX, l_euclidY;
    int l_weightYZ, l_weightY;

    ++lookupX;
    ++lookupY;
    ++lookupZ;

    for(int i = 0; i < lookupX; ++i)
    {
	l_weightYZ = i * lookupY * lookupZ;
	l_euclidX = t_squareNum(i*mt_scalingX);
	for(int j = 0; j < lookupY; ++j)
	{
	    l_weightY = j * lookupZ;
	    l_euclidY = t_squareNum(j*mt_scalingY);
	    for(int k = 0; k < lookupZ; ++k)
	    {

		if(mt_interpolateType == GAUSSIAN_INTERPOLATION)
		{
		    theLookupTable[l_weightYZ + l_weightY + k] = ((1/(mt_interpolateSigma * sqrt(2*3.14159265)))
								  * exp(-(l_euclidX + l_euclidY + t_squareNum(k*mt_scalingZ))
									/ (2*t_squareNum(mt_interpolateSigma))));
		}
		else if(mt_interpolateType == KAISERBESSEL_INTERPOLATION)
		{
		    theLookupTable[l_weightYZ + l_weightY + k] =
			t_kaiserBesselI0(mt_interpolateSigma * sqrt(1 - t_squareNum((i*mt_scalingX) / (mt_interpolateRadius)))) *
			t_kaiserBesselI0(mt_interpolateSigma *  sqrt(1 - t_squareNum((j*mt_scalingY) / (mt_interpolateRadius)))) *
			t_kaiserBesselI0(mt_interpolateSigma *  sqrt(1 - t_squareNum((k*mt_scalingZ) / (mt_interpolateRadius))))
			/
			t_cubeNum(mt_interpolateRadius);
		}
		else if(mt_interpolateType == LINEAR_INTERPOLATION)
		{
		    theLookupTable[l_weightYZ + l_weightY + k] =
			sqrt(l_euclidX + l_euclidY + t_squareNum(k*mt_scalingZ));
		}
		else
		{
		    if(i==0 && k==0 && j==0) std::cerr<<"Evidently you have no idea what you are doing, so I'm choosing KAISERBESSEL_INTERPOLATION for you."<<std::endl;
            
		    theLookupTable[l_weightYZ + l_weightY + k] =
			t_kaiserBesselI0(mt_interpolateSigma * sqrt(1 - t_squareNum((i*mt_scalingX) / mt_interpolateRadius))) *
			t_kaiserBesselI0(mt_interpolateSigma *  sqrt(1 - t_squareNum((j*mt_scalingY) / mt_interpolateRadius))) *
			t_kaiserBesselI0(mt_interpolateSigma *  sqrt(1 - t_squareNum((k*mt_scalingZ) / mt_interpolateRadius)))
			/
			t_cubeNum(mt_interpolateRadius);
		}
	    }
	}
    }
}


inline void RegridVolume::t_transformVoxelToWorld(double voxelX, double voxelY, double voxelZ,
						  const double step[3],
						  const double origin[3],
						  vtkMatrix4x4* rotation,
						  double output_world[4]) const
{
    //convert the image's voxel coordinate to the world coordinate where the
    //volume will be created.

    double voxel[4] = {voxelX * step[0] + origin[0], 
		       voxelY * step[1] + origin[1],
		       voxelZ * step[2] + origin[2],
		       1.0};

    rotation->MultiplyPoint(voxel, output_world);
}


inline int RegridVolume::t_getVolumeVoxelOffset(double x, double y, double z) const
{
    // convert a volume's coordinate into the volume's voxel location
    // in a 1D array
    return ((int)floor(t_getVolumeCoordZ(z))) * mt_volOffsetConstXY
	+ ((int)floor(t_getVolumeCoordY(y))) *  mt_volOffsetConstX
	+ ((int)floor(t_getVolumeCoordX(x))) * mt_voxelScalarComponents;
}

    
inline double RegridVolume::t_getVolumeCoordX(double x) const
{
    //converts the world X into volume X by switching it's origin
    //to the volume's and then changing it's scaling
    return (x - mt_minX) / mt_scalingX;
}


inline double RegridVolume::t_getVolumeCoordY(double y) const
{
    //converts the world Y into volume Y by switching it's origin
    //to the volume's and then changing it's scaling
    return (y - mt_minY) / mt_scalingY;
}


inline double RegridVolume::t_getVolumeCoordZ(double z) const
{
    //converts the world Z into volume Z by switching it's origin
    //to the volume's and then changing it's scaling
    return (z - mt_minZ) / mt_scalingZ;
}


inline double RegridVolume::t_squareNum(double num) const
{
    return num * num;
}


inline double RegridVolume::t_cubeNum(double num) const
{
    return num * num * num;
}

inline float RegridVolume::t_max(float vals[3]) const
{
    if(vals[0] < vals[1])
    {            
        if(vals[0] < vals[2])     
        {
            if(vals[1] < vals[2]) return vals[2];
            else return vals[1];
        }  
        else return vals[1];
    }
    else if(vals[1] < vals[2])         
    {
        if(vals[0] < vals[2]) return vals[2];
        else return vals[0];
    }
    else return vals[0];
}

inline double RegridVolume::t_kaiserBesselI0(double value) const
{
    //Some kaiser-bessel function imple. stolen from:
    //http://ccrma.stanford.edu/CCRMA/Courses/422/projects/kbd/kbdwindow.cpp
    if (value == 0.0)
    {
	return 1.0;
    }
    else
    {
	double z = t_squareNum(value);
	return -(z * (z * 
		      (z * (z * 
			    (z * (z * 
				  (z * (z * 
					(z * (z * 
					      (z * (z * 
						    (z * (z * 0.210580722890567e-22 + 0.380715242345326e-19 ) +
						     0.479440257548300e-16) + 0.435125971262668e-13 ) +
					       0.300931127112960e-10) + 0.160224679395361e-7  ) +
					 0.654858370096785e-5) + 0.202591084143397e-2  ) +
				   0.463076284721000e0) + 0.754337328948189e2   ) +
			     0.830792541809429e4) + 0.571661130563785e6   ) +
		       0.216415572361227e8) + 0.356644482244025e9   ) +
		 0.144048298227235e10)
	    /
	    (z * (z * (z - 0.307646912682801e4) + 0.347626332405882e7) - 0.144048298227235e10);
    }
}


inline double RegridVolume::t_saturation(double vals[3]) const
{
    //This function calculates the percentage saturation
    // (HighestVal(r,g,b) - LowestVal(r,g,b)) / HighestVal(r,g,b)

    if(vals[0] == vals[1] && vals[0] == vals[2])
    {
	return 0.0;
    }
    else if(vals[0] < vals[1])
    {            
	if(vals[0] < vals[2])     
	{
	    if(vals[1] < vals[2]) return (vals[2] - vals[0])/vals[2];
	    else return (vals[1] - vals[0])/vals[1];
	}  
	else return (vals[1] - vals[2])/vals[1];
    }
    else if(vals[1] < vals[2])         
    {
	if(vals[0] < vals[2]) return (vals[2] - vals[1])/vals[2];
	else return (vals[0] - vals[1])/vals[0];
    }
    else return (vals[0] - vals[2])/vals[0];
}
