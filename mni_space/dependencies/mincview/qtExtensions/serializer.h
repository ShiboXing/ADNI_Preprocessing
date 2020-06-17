//==========================================================================
//  Copyright (c) 2007-2010 IPL, BIC, MNI, McGill, Simon Drouin
//  
//==========================================================================
#ifndef __Serializer_h 
#define __Serializer_h

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <vector>
#include <utility>


// Description:
// This class is the base class for reading/writing xml files with
// settings of a program. Typically, to write a file, you create an
//
// SerializerWriter writer;
// writer.SetFilename("afile.xml");
// writer.Start();
// Serialize( &writer, "attributename", attributeValue );
// ...
// writer.BeginSection();
// ...
// Serialize( &writer, "anotherattributename", anotherattributeValue );
// ...
// writer.EndSection();
// ...
// Serialize( &writer, "anotherattributename", anotherattributeValue );
// writer.Finish()
//
// and you do the same to read a file. The Serializer class and its children
// where made so that we use the same code for reading and writing. The only
// thing that changes is the instanciated Serializer (SerializerWriter to write
// and SerializerReader to read). In some cases, if you have to execute different
// code in reading and writing cases, you can call the function IsReader() to
// know which one you are in.
//
// See also: SerializerReader SerializerWriter
class Serializer
{
    
public:
    
    Serializer() : m_document("configML")
    {
        m_root = m_document.createElement( "configuration" );
        m_document.appendChild( m_root );
        m_currentNode = m_root;
    }
    virtual ~Serializer() { } 
    
    virtual int IsReader() = 0;
    void SetFilename( const char * name ) { m_filename = name; }
    virtual bool Start() = 0;
    virtual bool Finish() = 0;
    virtual int BeginSection( const char * attrName ) = 0;
    virtual void EndSection() = 0;
    
    virtual void Serialize( const char * attrName, int & value ) =  0;
    virtual void Serialize( const char * attrName, bool & value ) =  0;
    virtual void Serialize( const char * attrName, double & value ) = 0;
    virtual void Serialize( const char * attrName, std::string & value ) = 0;
    virtual void Serialize( const char * attrName, QString & value ) = 0;
    virtual void Serialize( const char * attrName, int * value, int nbElements ) = 0;
    virtual void Serialize( const char * attrName, double * value, int nbElements ) = 0;
    
protected:
    
    std::string    m_filename;
    
    QDomDocument   m_document;
    QDomElement    m_root;
    QDomNode       m_currentNode;    
};



class SerializerWriter : public Serializer
{
    
public:
    
    SerializerWriter() {}
    ~SerializerWriter() {}
    
    virtual int IsReader()
    {
        return 0;
    }
    
    virtual bool Start()
    {
        m_currentNode = m_root;
        return true;
    }
    
    virtual bool Finish()
    {
        QFile file( m_filename.c_str() );
        if( !file.open( IO_WriteOnly ) )
          return false;

        QTextStream ts( &file );
        ts << m_document.toString();

        file.close();
        return true;
    }
    
    
    virtual int BeginSection( const char * attrName )
    {
        QDomElement elem = m_document.createElement( attrName );
        m_currentNode =  m_currentNode.appendChild( elem );
        return 1;
    }
    
    virtual void EndSection()
    {
        m_currentNode = m_currentNode.parentNode();
    }
    
    virtual void Serialize( const char * attrName, int & value )
    {
        QDomElement elem = m_document.createElement( attrName );
        elem.setAttribute( "value", QString::number( value ) );
        m_currentNode.appendChild( elem );
    }
    
    virtual void Serialize( const char * attrName, bool & value )
    {
        QDomElement elem = m_document.createElement( attrName );
        elem.setAttribute( "value", QString::number( value ) );
        m_currentNode.appendChild( elem );
    }
    
    virtual void Serialize( const char * attrName, double & value )
    {
        QDomElement elem = m_document.createElement( attrName );
        elem.setAttribute( "value", QString::number( value, 'e', 16 ) );
        m_currentNode.appendChild( elem );
    }
    
    virtual void Serialize( const char * attrName, std::string & value )
    {
        QDomElement elem = m_document.createElement( attrName );
        elem.setAttribute( "value", value.c_str() );
        m_currentNode.appendChild( elem );
    }
    
    virtual void Serialize( const char * attrName, QString & value )
    {
        QDomElement elem = m_document.createElement( attrName );
        elem.setAttribute( "value", value );
        m_currentNode.appendChild( elem );
    }
    
    virtual void Serialize( const char * attrName, int * value, int nbElements )
    {
        QDomElement elem = m_document.createElement( attrName );
        QString text;
        for( int i = 0; i < nbElements; i++ )
        {
            text += QString::number( value[i] ) + " ";
        }
        elem.setAttribute( "value", text );
        m_currentNode.appendChild( elem );
    }
    
    virtual void Serialize( const char * attrName, double * value, int nbElements )
    {
        QDomElement elem = m_document.createElement( attrName );
        QString text;
        for( int i = 0; i < nbElements; i++ )
        {
            text += QString::number( value[i], 'e', 16 ) + " ";
        }
        elem.setAttribute( "value", text );
        m_currentNode.appendChild( elem );
    }
};


class SerializerReader : public Serializer
{

public:
    
    SerializerReader() {}
    ~SerializerReader() {}
    
    virtual int IsReader()
    {
        return 1;
    }
    
    virtual bool Start()
    {
        QFile file( m_filename );
        
        if( !file.open( IO_ReadOnly ) )
            return false;

        if( !m_document.setContent( &file ) )
        {
            file.close();
            return false;
        }
        file.close();
        
        m_root = m_document.documentElement();
        if( m_root.tagName() != "configuration" )
        {
            // manage error!!!
            return false;
        }
        m_currentNode = m_root;
        return true;
    }
    
    virtual bool Finish()
    {
        return true;
    }
    
    virtual int BeginSection( const char * attrName )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            m_currentNode = variableNode;
            return 1;
        }
        return 0;
    }
    
    virtual void EndSection( )
    {
        m_currentNode = m_currentNode.parentNode();
    }
    
    virtual void Serialize( const char * attrName, int & value )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            QString variableValueString = variableNode.toElement().attribute( "value" );
            if( !variableValueString.isNull() )
            {
                value = variableValueString.toInt();
            }
        }
    }
    
    virtual void Serialize( const char * attrName, bool & value )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            QString variableValueString = variableNode.toElement().attribute( "value" );
            if( !variableValueString.isNull() )
            {
                value = variableValueString.toInt();
            }
        }
    }
    
    virtual void Serialize( const char * attrName, double & value )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            QString variableValueString = variableNode.toElement().attribute( "value" );
            if( !variableValueString.isNull() )
            {
                value = variableValueString.toDouble();
            }
        }
    }
    
    virtual void Serialize( const char * attrName, std::string & value )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            QString variableValueString = variableNode.toElement().attribute( "value" );
            if( !variableValueString.isNull() )
            {
                value = variableValueString.ascii();
            }
        }
    }
    
    virtual void Serialize( const char * attrName, QString & value )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            QString variableValueString = variableNode.toElement().attribute( "value" );
            if( !variableValueString.isNull() )
            {
                value = variableValueString.ascii();
            }
        }
    }
    
    virtual void Serialize( const char * attrName, int * value, int nbElements )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            QString variableValueString = variableNode.toElement().attribute( "value" );
            if( !variableValueString.isNull() )
            {
                QStringList list = QStringList::split( ' ', variableValueString );
                QStringList::iterator it = list.begin();
                QStringList::iterator itEnd = list.end();
                int i = 0;
                while( it != itEnd && i < nbElements )
                {
                    value[i] = (*it).toInt();
                    ++it;
                    ++i;
                }
            }
        }
    }
    
    virtual void Serialize( const char * attrName, double * value, int nbElements )
    {
        QDomNode variableNode = m_currentNode.namedItem( attrName );
        if( !variableNode.isNull() )
        {
            QString variableValueString = variableNode.toElement().attribute( "value" );
            if( !variableValueString.isNull() )
            {
                QStringList list = QStringList::split( ' ', variableValueString );
                QStringList::iterator it = list.begin();
                QStringList::iterator itEnd = list.end();
                int i = 0;
                while( it != itEnd && i < nbElements )
                {
                    value[i] = (*it).toDouble();
                    ++it;
                    ++i;
                }
            }
        }
    }
   
};


//========================================================================
// Helper functions
//========================================================================

// Description:
// This is the main serialization function that will be called for every basic type
// supported by the serializers. It is there to provide a common syntax to serialize
// any type of data (even basic types).
template< class T > void Serialize( Serializer * serial, const char * attrName, T & value )
{
    serial->Serialize( attrName, value );
}


template< class T > void Serialize( Serializer * serial, const char * attrName, T & value, int nbElements )
{
    serial->Serialize( attrName, value, nbElements );
}


// Description:
// Specialization function to serialize std::vector containers
template< class T > void Serialize( Serializer * serial, const char * attrName, std::vector<T> & value )
{
    if( serial->BeginSection( attrName ) )
    {
        int numberOfElements = value.size();
        Serialize( serial, "NumberOfElements", numberOfElements );
        if( serial->IsReader() )
        {
            value.resize( numberOfElements );
        }
            
        typename std::vector<T>::iterator it = value.begin();
        int i = 0;
        while( it != value.end() )
        {
            QString elemName = QString( "Element_%1" ).arg(i);
            Serialize( serial, elemName, *(it) );
            ++i;
            ++it;        
        }
        serial->EndSection();
    }
}


template< class T, class R > void Serialize( Serializer * serial, const char * attrName, std::pair<T,R> & value )
{
    if( serial->BeginSection( attrName ) )
    {
        Serialize( serial, "First", value.first );
        Serialize( serial, "Second", value.second );
        serial->EndSection();
    }
}


// Description:
// This macro should be used in header files of classes you want
// to be able to serialize. These classes have to implement
// a function with this signature:
//
// void Serialize( Serializer * serial )
//
#define ObjectSerializationMacro( className ) \
void Serialize( Serializer * serial, const char * attrName, className * value ) \
{ \
    if( serial->BeginSection( attrName ) ) \
    { \
        value->Serialize( serial ); \
        serial->EndSection(); \
    } \
} \
void Serialize( Serializer * serial, const char * attrName, className & value ) \
{ \
    if( serial->BeginSection( attrName ) ) \
    { \
        value.Serialize( serial ); \
        serial->EndSection(); \
    } \
}

#define ObjectSerializationHeaderMacro( className ) \
void Serialize( Serializer * serial, const char * attrName, className * value ); \
void Serialize( Serializer * serial, const char * attrName, className & value );

#endif
