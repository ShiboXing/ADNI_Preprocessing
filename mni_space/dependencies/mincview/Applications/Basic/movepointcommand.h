#ifndef TAG_MOVEPOINTCOMMAND_H_
#define TAG_MOVEPOINTCOMMAND_H_

#include "vtkCommand.h"

class PointsObject;

class MovePointCommand : public vtkCommand
{
            
public:
    static MovePointCommand *New() { return new MovePointCommand; }
    void Delete() { delete this; }
        
    virtual void Execute(vtkObject *caller, unsigned long l, void *callData);
    virtual void SetPointsObject(PointsObject *pts);
        
protected:
        
    MovePointCommand();
    virtual ~MovePointCommand();
    PointsObject *m_points;

};

#endif //TAG_MOVEPOINTCOMMAND_H_
