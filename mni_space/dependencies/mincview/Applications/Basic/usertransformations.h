/*=========================================================================

  Program:   Igns global tools
  Module:    $RCSfile: usertransformations.h,v $
  Language:  C++
  Date:      $Date: 2010-04-28 19:06:03 $
  Version:   $Revision: 1.11 $

  Copyright (c) Anka, IPL, BIC, MNI, McGill
  All rights reserved.
  
=========================================================================*/
#ifndef TAG_USERTRANSFORMATIONS_H_
#define TAG_USERTRANSFORMATIONS_H_

#include <vector>
#include <qobject.h>
#include <qstring.h>
#include "vtkTransform.h"
#include "serializer.h"
#include "expertmode.h"

// I define here names of 2 pre-set transformations that may be initially applied
// on top of the transformation from the data file
// as the name Identity confuses some people, so I'll display From Data

#define IGNS_IDENTITY_TRANSFORM "From Data" 
#define IGNS_REGISTRATION "Registration"
#define IGNS_X_PLUS_1     "x+1"
#define IGNS_Y_PLUS_1     "y+1"
#define IGNS_Z_PLUS_1     "z+1"
#define IGNS_X_MINUS_1    "x-1"
#define IGNS_Y_MINUS_1    "y-1"
#define IGNS_Z_MINUS_1    "z-1"
#define IGNS_X_PLUS_5     "x+5"
#define IGNS_Y_PLUS_5     "y+5"
#define IGNS_Z_PLUS_5     "z+5"
#define IGNS_X_MINUS_5    "x-5"
#define IGNS_Y_MINUS_5    "y-5"
#define IGNS_Z_MINUS_5    "z-5"
#define IGNS_ABSOLUTE_TRANSFORM "Absolute"
#define IGNS_INCREMENTAL_TRANSFORM "Incremental"

class vtkTransform;
class vtkMatrix4x4;

enum USER_TRANSFORM_TYPE {ABSOLUTE_TRANSFORM, INCREMENTAL_TRANSFORM, UNKNOWN_TRANSFORM_TYPE};

struct TransformParams
{
    TransformParams();
    ~TransformParams();
    void Serialize( Serializer * serializer );
    QString name;
    int predefinedTransform;
    int type;
    int referenceCounter;
    vtkTransform *transform;
};

ObjectSerializationHeaderMacro( TransformParams );
    

class UserTransformations : public QObject
{

    Q_OBJECT

public:

    UserTransformations();
    virtual ~UserTransformations();
    
    void SetRegistrationTransform(vtkLinearTransform *);
    vtkTransform * GetRegistrationTransform();
    void GetTransformList(QStringList &);
    bool TransformIsPredefined(int index );
    bool TransformIsPredefined(QString name );
    int GetTransformType(int index);
    int GetTransformType(QString name);
    int GetTransformIndex(QString name);
    QString GetTransformName(int index);
    vtkMatrix4x4 * GetTransformMatrix(int index);
    vtkMatrix4x4 * GetTransformMatrix(QString name);
    void AddUserTransform(const QString &name, int type, bool postMultiply, vtkMatrix4x4 *mat);
    void DeleteUserTransform(const QString &name);
    void UpdateUserTransform(const QString &oldName,const QString &name, int type, bool postMultiply, vtkMatrix4x4 *mat);
    vtkTransform *GetUserTransform(int index);
    vtkTransform *GetUserTransform(QString name); 
    void Serialize( Serializer * ser );
    void SetRegistrationFlag(bool flag) {m_registered = flag;}
    bool GetRegistrationFlag() {return m_registered;}
    void SetExpertMode(ExpertMode *mode) {m_expertMode = mode;}
    int GetExpertLevel();
    int GetReferenceCounter(int index);
    void IncrementReferenceCounter(int index);
    void DecrementReferenceCounter(int index);
    int GetReferenceCounter(QString name);
    void IncrementReferenceCounter(QString name);
    void DecrementReferenceCounter(QString name);
    int GetNumberOfPredefinedTransforms() {return m_numPredefined;}
    
signals:
    void TransformsChanged();    
    
protected:

    bool m_registered;
    int m_numPredefined;
    typedef std::vector<TransformParams*> UserTransformList;
    UserTransformList m_userTransforms;
private:
    ExpertMode *m_expertMode;
    
};

ObjectSerializationHeaderMacro( UserTransformations );

#endif //TAG_USERTRANSFORMATIONS_H_
