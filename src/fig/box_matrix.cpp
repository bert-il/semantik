// Thomas Nagy 2007-2016 GPLV3

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextList>
#include <QClipboard>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtDebug>
#include <QAction>
#include <QTextDocument>
#include \
  "matrix_dialog.h"
#include "box_matrix.h"
#include "data_item.h"
 #include "res:zable.h"
#include "box_view.h"
 #include "box_link.h"
#include "sem_mediator.h"
#include "mem_box.h"

#include <QFont>

#define PAD 2
#define MIN_FORK_SIZE 30

box_matrix::box_matrix(box_view* view, int id) : box_item(view, id)
{
	QFont font = doc.defaultFont();
	font.setPointSize(font.pointSize() - 2);
	doc.setDefaultFont(font);

	setZValue(80);
	update_size();
}

void box_matrix::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();

	QRectF l_oRect = boundingRect().adjusted(PAD, PAD, -PAD, -PAD);

	QPen l_oPen = QPen(Qt::SolidLine);
	l_oPen.setColor(Qt::black);
	l_oPen.setWidth(1);
	painter->setPen(l_oPen);


	qreal l_iHref = l_oRect.top();
	foreach (int l_iHoff, m_oBox->m_oRowSizes)
	{
		l_iHref += l_iHoff;
		painter->drawLine(l_oRect.left(), l_iHref, l_oRect.right(), l_iHref);
	}
	l_iHref = l_oRect.left();
	foreach (int l_iHoff, m_oBox->m_oColSizes)
	{
		l_iHref += l_iHoff;
		painter->drawLine(l_iHref, l_oRect.top(), l_iHref, l_oRect.bottom());
	}

	painter->drawRect(l_oRect);

	QRectF l_oR2 = boundingRect();
	qreal l_oX = l_oR2.right();
	qreal l_oY = l_oR2.bottom();

	if (isSelected())
	{
		QRectF l_oR(-8, -8, 6, 6);
		painter->setBrush(QColor("#FFFF00"));

		qreal l_i = 6 - PAD;
		foreach (int l_iSize, m_oBox->m_oRowSizes) {
			l_i += l_iSize;
			painter->drawRect(l_oR.translated(l_oX, l_i));
		}

		l_i = 6 - PAD;
		foreach (int l_iSize, m_oBox->m_oColSizes) {
			l_i += l_iSize;
			painter->drawRect(l_oR.translated(l_i, l_oY));
		}

		painter->drawRect(l_oR.translated(l_oX, l_oY));
	}

	painter->restore();
}

void box_matrix::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
	m_oLastPressPoint = e->pos();
	QRectF l_oR(-10, -10, 8, 8);

	QRectF l_oR2 = boundingRect();
	qreal l_oX = l_oR2.right();
	qreal l_oY = l_oR2.bottom();

	qreal l_i = 6 - PAD;
	int i = 0;
	foreach (int l_iSize, m_oBox->m_oRowSizes) {
		l_i += l_iSize;
		if (l_oR.translated(l_oX, l_i).contains(m_oLastPressPoint))
		{
			m_iLastSize = l_iSize;
			m_iMovingRow = i;
			m_iMovingCol = -1;
			setFlags(ItemIsSelectable | ItemSendsGeometryChanges);
			m_bMoving = true;
			QGraphicsRectItem::mousePressEvent(e);
			return;
		}
		i += 1;
	}

	i = 0;
	l_i = 6 - PAD;
	foreach (int l_iSize, m_oBox->m_oColSizes) {
		l_i += l_iSize;
		if (l_oR.translated(l_i, l_oY).contains(m_oLastPressPoint))
		{
			m_iLastSize = l_iSize;
			m_iMovingRow = -1;
			m_iMovingCol = i;
			setFlags(ItemIsSelectable | ItemSendsGeometryChanges);
			m_bMoving = true;
			QGraphicsRectItem::mousePressEvent(e);
			return;
		}
		i += 1;
	}

	if (l_oR.translated(l_oX, l_oY).contains(m_oLastPressPoint))
	{
		m_iMovingRow = m_iMovingCol = -1;
		setFlags(ItemIsSelectable | ItemSendsGeometryChanges);
		m_bMoving = true;
		QGraphicsRectItem::mousePressEvent(e);
		return;
	}
	QGraphicsRectItem::mousePressEvent(e);
}

void box_matrix::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	if (m_bMoving)
	{
		QPointF np = e->pos();
		int x = np.x() - m_oLastPressPoint.x();
		int y = np.y() - m_oLastPressPoint.y();

		if (m_iMovingRow == -1 && m_iMovingCol == -1) {
			m_iWW = m_oBox->m_iWW + x;
			if (m_iWW < 2 * GRID) m_iWW = 2 * GRID;
			m_iWW = grid_int(m_iWW);

			int l_iWidth = 0;
			foreach (int l_iSize, m_oBox->m_oColSizes) {
				l_iWidth += l_iSize;
			}
			if (m_iWW < l_iWidth + 2 * GRID) {
				m_iWW = l_iWidth + 2 * GRID;
			}

			m_iHH = m_oBox->m_iHH + y;
			if (m_iHH < 2 * GRID) m_iHH = 2 * GRID;
			m_iHH = grid_int(m_iHH);

			int l_iHeight = 0;
			foreach (int l_iSize, m_oBox->m_oRowSizes) {
				l_iHeight += l_iSize;
			}
			if (m_iHH < l_iHeight + 2 * GRID) {
				m_iHH = l_iHeight + 2 * GRID;
			}

			prepareGeometryChange();
			setRect(0, 0, m_iWW + 2*PAD, m_iHH + 2*PAD);
			m_oView->message(m_oView->trUtf8("Last row: %2px, last column: %3px (size: %4 x %5)").arg(
				QString::number(m_iHH - l_iHeight),
				QString::number(m_iWW - l_iWidth),
				QString::number(m_iWW),
				QString::number(m_iHH)
			), 5000);
		}
		else if (m_iMovingRow > -1)
		{
			int l_iSize = grid_int(m_iLastSize + y);
			if (l_iSize < 2 * GRID) {
				l_iSize = 2 * GRID;
			}
			m_oBox->m_oRowSizes[m_iMovingRow] = l_iSize;
			int l_iNewHeight = m_oBox->m_iHH + l_iSize - m_iLastSize;
			setRect(0, 0, m_oBox->m_iWW + 2 * PAD, l_iNewHeight + 2 * PAD);
			m_oView->message(m_oView->trUtf8("Row %1: %2px (size: %3 x %4)").arg(
				QString::number(m_iMovingRow + 1),
				QString::number(l_iSize),
				QString::number(m_iWW),
				QString::number(l_iNewHeight)
			), 5000);
		}
		else if (m_iMovingCol > -1)
		{
			int l_iSize = grid_int(m_iLastSize + x);
			if (l_iSize < 2 * GRID) {
				l_iSize = 2 * GRID;
			}
			m_oBox->m_oColSizes[m_iMovingCol] = l_iSize;
			int l_iNewWidth = m_oBox->m_iWW + l_iSize - m_iLastSize;
			setRect(0, 0, l_iNewWidth +  2 * PAD, m_oBox->m_iHH +  2 * PAD);
			m_oView->message(m_oView->trUtf8("Column %1: %2px (size: %3 x %4)").arg(
				QString::number(m_iMovingCol + 1),
				QString::number(l_iSize),
				QString::number(l_iNewWidth),
				QString::number(m_iHH)
			), 5000);
		}

		m_oChain->setPos(boundingRect().right() + 3, 0);
		update();
		update_links();
	}
	else
	{
		QGraphicsRectItem::mouseMoveEvent(e);
	}
}

void box_matrix::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
	if (m_bMoving)
	{
		m_bMoving = false;
		setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
		if (m_iWW != m_oBox->m_iWW || m_iHH != m_oBox->m_iHH)
		{
			mem_size_box *mem = new mem_size_box(m_oView->m_oMediator, m_oView->m_iId);
			mem->prev_values[m_oBox] = QRect(m_oBox->m_iXX, m_oBox->m_iYY, m_oBox->m_iWW, m_oBox->m_iHH);
			mem->next_values[m_oBox] = QRect(m_oBox->m_iXX, m_oBox->m_iYY, m_iWW, m_iHH);
			mem->apply();
		} else if (m_iMovingRow > -1) {
			mem_matrix *mem = new mem_matrix(m_oView->m_oMediator, m_oView->m_iId);
			mem->init(m_oBox);
			mem->m_oOldRowSizes[m_iMovingRow] = m_iLastSize;
			mem->m_iNewHH += m_oBox->m_oRowSizes[m_iMovingRow] - m_iLastSize;
			mem->apply();
		} else if (m_iMovingCol > -1) {
			mem_matrix *mem = new mem_matrix(m_oView->m_oMediator, m_oView->m_iId);
			mem->init(m_oBox);
			mem->m_oOldColSizes[m_iMovingCol] = m_iLastSize;
			mem->m_iNewWW += m_oBox->m_oColSizes[m_iMovingCol] - m_iLastSize;
			mem->apply();
		}
	}
	else
	{
		QGraphicsRectItem::mouseReleaseEvent(e);
	}
}

void box_matrix::update_size() {
	m_iWW = m_oBox->m_iWW;
	m_iHH = m_oBox->m_iHH;

	setRect(0, 0, m_iWW + 2*PAD, m_iHH + 2*PAD);
	m_oChain->setPos(boundingRect().right() + 3, 0);

	update_links();
}

void box_matrix::properties()
{
	matrix_dialog l_o(m_oView);
	l_o.m_oRows->setValue(m_oBox->m_oRowSizes.length() + 1);
	l_o.m_oCols->setValue(m_oBox->m_oColSizes.length() + 1);
	if (l_o.exec() == QDialog::Accepted) {
		mem_matrix *mem = new mem_matrix(m_oView->m_oMediator, m_oView->m_iId);
		mem->init(m_oBox);

		while (mem->m_oNewRowSizes.size() < l_o.m_oRows->value() - 1)
		{
			int l_iTmp = mem->m_iNewHH;
			foreach (int l_i, mem->m_oNewRowSizes) {
				l_iTmp -= l_i;
			}
			mem->m_oNewRowSizes.push_back(l_iTmp);
			mem->m_iNewHH += l_iTmp;
		}
		while (mem->m_oNewRowSizes.size() > l_o.m_oRows->value() - 1)
		{
			mem->m_iNewHH = 0;
			foreach (int l_i, mem->m_oNewRowSizes) {
				mem->m_iNewHH += l_i;
			}
			mem->m_oNewRowSizes.removeLast();
		}

		while (mem->m_oNewColSizes.size() < l_o.m_oCols->value() - 1)
		{
			int l_iTmp = mem->m_iNewWW;
			foreach (int l_i, mem->m_oNewColSizes) {
				l_iTmp -= l_i;
			}
			mem->m_oNewColSizes.push_back(l_iTmp);
			mem->m_iNewWW += l_iTmp;
		}
		while (mem->m_oNewColSizes.size() > l_o.m_oCols->value() - 1)
		{
			mem->m_iNewWW = 0;
			foreach (int l_i, mem->m_oNewColSizes) {
				mem->m_iNewWW += l_i;
			}
			mem->m_oNewColSizes.removeLast();
		}

		mem->apply();
	}
}


