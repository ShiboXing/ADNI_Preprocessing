#ifndef TAG_PICKCOMMAND_H_
#define TAG_PICKCOMMAND_H_

#include <qobject.h>
#include "vtkCommand.h"
#include "vtkReal.h"

class vtkCellPicker;

class PickCommand : public QObject, public vtkCommand
{
    Q_OBJECT
            
public:
    static PickCommand *New() { return new PickCommand; }
    void Delete() { delete this; }
    
    virtual void SetPicker( vtkCellPicker *pick );
    
    virtual void Execute(vtkObject *caller, unsigned long l, void *callData);
    
signals:

     void NewPointAdded(vtkReal *pt);
     
protected:
    
    vtkCellPicker *picker;
    
    PickCommand();
    virtual ~PickCommand();
    
};

#endif //TAG_PICKCOMMAND_H_
