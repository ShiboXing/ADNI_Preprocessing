#ifndef QtExtListViewItem_h_
#define QtExtListViewItem_h_

#include <qlistview.h>

class QPainter;
class QFont;
class QColor;
class QColorGroup;
class QString;
class QFontMetrics;

class QtExtListViewItem : public QListViewItem
{
    public:
        // reimplement all the constructors of the QListViewItem
        QtExtListViewItem(QListView *parent): QListViewItem(parent)
        {init();}
        QtExtListViewItem(QListViewItem *parent): QListViewItem(parent)
        {init();}
        QtExtListViewItem(QListView *parent, QListViewItem *after): QListViewItem(parent, after)
        {init();}
        QtExtListViewItem(QListViewItem *parent, QListViewItem *after)
            : QListViewItem(parent, after)
        {init();}
        
        QtExtListViewItem(QListView *parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )
            :QListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
        {init();}
        
        QtExtListViewItem(QListViewItem *parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )
            :QListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
        {init();}
        
        QtExtListViewItem(QListView *parent, QListViewItem *after, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )
            :QListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
        {init();}
        
        QtExtListViewItem(QListViewItem *parent, QListViewItem *after, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )
            :QListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
        {init();}
        
        QtExtListViewItem(QListView *parent, const QString &label1, const QString &label2)
            : QListViewItem(parent, label1, label2)
        {init();}
    
        QtExtListViewItem(QListViewItem *parent, const QString &label1, const QString &label2)
            : QListViewItem(parent, label1, label2)
        {init();}
    
        
        // Methods
        virtual void paintCell(QPainter *painter, const QColorGroup &cg,
                       int column, int width, int align);
        virtual int width(const QFontMetrics &fm, const QListView *lv, int column) const;
    
        void init();
        QFont getFont() const {return Font;}
        void setFont(const QFont &font) {Font = font; fontSet = true;}
        QColor getBackgroundColor() const {return BackgroundColor;}
        void setBackgroundColor(const QColor &color) {BackgroundColor = color; backgroundColorSet = true;}
        QColor getTextColor() const {return TextColor;}
        void setTextColor(const QColor &color) {TextColor = color; textColorSet = true;}
    
    private:
        QFont Font;
        QColor BackgroundColor;
        QColor TextColor;
        bool fontSet;
        bool backgroundColorSet;
        bool textColorSet;
};

#endif //QtExtListViewItem_h_
