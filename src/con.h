// Thomas Nagy 2007-2009 GPLV3

#include <QtGlobal>

#ifndef CON
#define CON

#define notr(x) QString(x)

#define xp(a, b, d) a->setItem(b, 0, new QStandardItem(d));
#define yp(a, b, d) a->setItem(b, 1, new QStandardItem(d));


#define GRID_VALUE 10.
#define nof(x, y) ((int) ((y) * ((int) ((x) / (y)))))
#define int_val2(x)  (nof(x, GRID_VALUE/2.))
#define int_val(x)   (nof(x, GRID_VALUE))

#define mu(x)   #x

#define fu(x, y)  mu(x##y)

#define min_offset 15


#define NO_ITEM 0
#define NO_VIEW 0

#define mix(x, y) {x^=y;y^=x;x^=y;}

 #define CONST const

#ifdef Q_OS_DARWIN
#define _TAILLETABLE_ 300
#define _NBRECT_ 500
#define _RECTMAXSIZE_ 9999999
#define _RATIO_GAP_ 10.0
#define _R_ 120.0
#define _D_ 4.0
#define _ROWS_ 300
#define _LONG_ "long"
#define _LARG_ "larg"
#define _POSX_ "posx"
#define _POSY_ "posy"
#define _ORI_  "ori"
#endif

#define moc(x) fu(x, moc)

#incl\
ude \
"aux.h"

#include <QHash>
#include <QVariant>

#define hash_params QHash<int, QVariant>

qreal _min(qreal a, qreal b);
qreal _max(qreal a, qreal b);

#define data_parent 0
#define data_id 1
#define data_id1 2
#define data_id2 3
#define data_orig 4
#define data_commande 5
#define data_type 6 // change the view stuff


#define view_canvas 1
#define view_linear 2
#define view_text 3
#define view_img 4
#define view_diag 5
#define view_table 6

#define EMPTY_TEXT "Empty"

#include <QCoreApplication>
//#define trUtf8(x) QCoreApplication::translate(NULL, x)

#define fillloc(x) xp(x, 0, "exclude_pic"); yp(x, 0, trUtf8("exclude the picture")); xp(x, 1, "exclude_pic.html"); xp(x, 2, "exclude_pic.pdflatex"); xp(x, 3, "exclude_table"); yp(x, 3, trUtf8("exclude the table")); xp(x, 4, "exclude_table.html"); xp(x, 5, "exclude_table.pdflatex"); xp(x, 6, "make_slide.s5"); yp(x, 6, trUtf8("fit the item on one slide")); xp(x, 7, "piccaptionstyle.html"); yp(x, 7, trUtf8("picture caption")); xp(x, 8, "picdim.pdflatex"); yp(x, 8, trUtf8("picture dimension in pdf files")); xp(x, 9, "picdivstyle.html"); yp(x, 9, trUtf8("html div style: style=\"width: 99px;\"")); xp(x, 10, "picstyle.html"); yp(x, 10, trUtf8("html picture style: style=\"width: 99px;\"")); xp(x, 11, "skip_slide.s5"); yp(x, 11, trUtf8("do not use this item for generating slides")); 

#define fillglo(x) xp(x, 0, "babel"); yp(x, 0, trUtf8("latex language: english (frenchb, )")); xp(x, 1, "babel.beamer"); xp(x, 2, "babel.pdflatex"); xp(x, 3, "command"); yp(x, 3, trUtf8("command to launch (%s is the resulting file)")); xp(x, 4, "command.beamer"); yp(x, 4, trUtf8("cd `echo %s | sed s/main.tex//` && ./waf configure && waf --view")); xp(x, 5, "command.html"); yp(x, 5, trUtf8("firefox %s")); xp(x, 6, "command.odp"); yp(x, 6, trUtf8("ooimpress %s")); xp(x, 7, "command.odt"); yp(x, 7, trUtf8("oowriter %s")); xp(x, 8, "command.pdflatex"); yp(x, 8, trUtf8("cd `echo %s | sed s/main.tex//` && ./waf configure && waf --view")); xp(x, 9, "command.s5"); yp(x, 9, trUtf8("firefox %s")); xp(x, 10, "doc_author"); yp(x, 10, trUtf8("document author")); xp(x, 11, "doc_author_off"); yp(x, 11, trUtf8("turn off the document author (latex)")); xp(x, 12, "doc_class"); yp(x, 12, trUtf8("document class (latex: a4, report, book)")); xp(x, 13, "doc_company"); yp(x, 13, trUtf8("company")); xp(x, 14, "doc_date"); yp(x, 14, trUtf8("document date")); xp(x, 15, "doc_date_off"); yp(x, 15, trUtf8("turn of the document date (latex)")); xp(x, 16, "doc_place"); yp(x, 16, trUtf8("location of the presentation")); xp(x, 17, "doc_title"); yp(x, 17, trUtf8("document title")); xp(x, 18, "doc_title_off"); yp(x, 18, trUtf8("turn off the document title (latex)")); xp(x, 19, "footer_c.pdflatex"); yp(x, 19, trUtf8("footer center (pdflatex)")); xp(x, 20, "footer_l.pdflatex"); yp(x, 20, trUtf8("footer left (pdflatex)")); xp(x, 21, "footer_off.pdflatex"); yp(x, 21, trUtf8("turn off the fancy output")); xp(x, 22, "footer_r.pdflatex"); yp(x, 22, trUtf8("footer right")); xp(x, 23, "header_c.pdflatex"); yp(x, 23, trUtf8("header center (pdflatex)")); xp(x, 24, "header_l.pdflatex"); yp(x, 24, trUtf8("header left")); xp(x, 25, "header_off.pdflatex"); xp(x, 26, "header_r.pdflatex"); yp(x, 26, trUtf8("header right")); xp(x, 27, "oolang"); yp(x, 27, trUtf8("openoffice.org language (fr_FR)")); xp(x, 28, "oolang.odp"); yp(x, 28, trUtf8("openoffice.org presentation language")); xp(x, 29, "oolang.odt"); yp(x, 29, trUtf8("openoffice.org text language")); xp(x, 30, "theme.beamer"); yp(x, 30, trUtf8("Warsaw (JuanLesPins, Montpellier, )")); 


#define aegy do { while(1){} } while (0)

//#define nono break
//while (1) { nono; }

#define errmsg "im sorry dave, im afraid i cant do that"

#define gratype(x) (QGraphicsItem::UserType + x)
#define CANVAS_ITEM_T gratype(12301)
#define CANVAS_FLAG_T gratype(12302)
#define CANVAS_LINK_T gratype(12303)
#define CANVAS_SORT_T gratype(12304)

/* end - con.h */
#endif

