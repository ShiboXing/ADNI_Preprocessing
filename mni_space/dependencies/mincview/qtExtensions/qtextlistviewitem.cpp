#include "qtextlistviewitem.h"

#include <qfont.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qstring.h>
#include <qfontmetrics.h>

void QtExtListViewItem::init()
{
    fontSet = false;
    backgroundColorSet = false;
    textColorSet = false;
}

void QtExtListViewItem::paintCell( QPainter *painter, const QColorGroup &cg, int column, int width, int align)
{
    painter->save();
    
    if (fontSet)
        painter->setFont(Font);
    
    QColorGroup grp(cg);
    if (backgroundColorSet)
        grp.setColor(QColorGroup::Base, BackgroundColor);
    if (textColorSet)
    {
        grp.setColor(QColorGroup::Text, TextColor);
        grp.setColor(QColorGroup::HighlightedText, TextColor);
    }
    
    QListViewItem::paintCell(painter, grp, column, width, align);
    painter->restore();
}
    

int QtExtListViewItem::width(const QFontMetrics &fm, const QListView *lv, int column) const
{
    if (fontSet)
    {
        QFontMetrics fm2(Font);
        return QListViewItem::width(fm2, lv, column);
    }
    else
    {
        return QListViewItem::width(fm, lv, column);
    }
}
