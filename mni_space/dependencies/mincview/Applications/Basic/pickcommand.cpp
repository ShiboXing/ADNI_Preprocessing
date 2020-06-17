#include <iostream>

#include "vtkCellPicker.h"
#include "vtkActor2D.h"
#include "pickcommand.h"


PickCommand::PickCommand()
{
    this->picker = 0; 
}

PickCommand::~PickCommand()
{
    if (this->picker)
        this->picker->UnRegister( this );   
}

void PickCommand::SetPicker( vtkCellPicker *pick ) 
{
    if( this->picker == pick )
        return;
    if (this->picker)
        this->picker->UnRegister(this);
    this->picker = pick; 
    if (this->picker)
        this->picker->Register( this );
}

void PickCommand::Execute(vtkObject *caller, unsigned long l, void *callData)
{  
    if ( this->picker->GetCellId() >= 0 )
    {
        vtkReal pickPos[3];
        this->picker->GetPickPosition( pickPos );
        emit NewPointAdded(&pickPos[0]);
   }
}

