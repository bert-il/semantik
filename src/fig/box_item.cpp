// Thomas Nagy 2007-2016 GPLV3

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QInputDialog>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>
#include <QTextList>
#include <QGraphicsSceneMouseEvent>
#include <QLinearGradient>
#include <QClipboard>
#include <QPainter>
#include <QtDebug>
#include <QAction>
#include <QTextDocument>
#include "box_item.h"
#include "box_view.h"
 #include "box_link.h"
#include "data_item.h"
#include "sem_mediator.h"
#include "mem_box.h"

#define PAD 2

box_item::box_item(box_view* i_oParent, int i_iId) : QGraphicsRectItem(), connectable(), editable(), m_oView(i_oParent)
{
	m_iId = i_iId;
	m_bMoving = false;

	m_oItem = m_oView->m_oMediator->m_oItems[m_oView->m_iId];
	m_oBox = m_oItem->m_oBoxes[m_iId];
	Q_ASSERT(m_oBox);

	i_oParent->scene()->addItem(this);
	m_oChain = new box_chain(i_oParent);
	m_oChain->setParentItem(this);

	setCacheMode(QGraphicsItem::DeviceCoordinateCache);

	update_size();
	setZValue(100);
	setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
}

box_item::~box_item()
{
	delete m_oChain;
}

void box_item::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QRectF l_oRect = boundingRect().adjusted(PAD, PAD, -PAD, -PAD);
	painter->setFont(scene()->font());
	doc.setDefaultFont(scene()->font());

	QPen l_oPen = QPen(Qt::SolidLine);

	l_oPen.setColor(Qt::black);
	if (isSelected()) l_oPen.setStyle(Qt::DotLine);
	l_oPen.setCosmetic(false);
	l_oPen.setWidth(1);

	painter->setPen(l_oPen);

	QColor bc(m_oBox->color);
	if (m_oView->m_bDisableGradient)
	{
		painter->setBrush(bc);
	}
	else
	{
		QLinearGradient linearGradient(0, 0, l_oRect.width(), 0);
		linearGradient.setColorAt(0.0, bc);
		linearGradient.setColorAt(1.0, bc.darker(GRADVAL));
		painter->setBrush(linearGradient);
	}


	painter->drawRoundRect(l_oRect, 20, 20);
	if (isSelected())
	{
		l_oPen.setStyle(Qt::SolidLine);
		painter->setPen(l_oPen);
		painter->setBrush(QColor("#FFFF00"));
		QRectF l_oR2(l_oRect.bottomRight() - QPointF(6, 6), l_oRect.bottomRight());
		painter->drawRect(l_oR2);

		/*painter->setBrush(Qt::green);
		l_oR2.setY(PAD);
		l_oR2.setHeight(6);
		painter->drawRect(l_oR2);*/
	}

	QAbstractTextDocumentLayout::PaintContext ctx;
	ctx.palette = QApplication::palette("QTextControl");
	ctx.palette.setColor(QPalette::Text, Qt::black); // white on black kde themes
	QAbstractTextDocumentLayout * lay = doc.documentLayout();
	qreal yoff = lay->documentSize().height();

	painter->translate(OFF, OFF + (m_iHH - 2 * OFF - yoff) / 2.);
	lay->draw(painter, ctx);
}

void box_item::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
	m_oLastPressPoint = e->pos();
	if (m_oLastPressPoint.x() > m_iWW - GRID && m_oLastPressPoint.y() > m_iHH - GRID )
	{
		setFlags(ItemIsSelectable | ItemSendsGeometryChanges);
		m_bMoving = true;
	}
	QGraphicsRectItem::mousePressEvent(e);
}

void box_item::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	if (m_bMoving)
	{
		QPointF np = e->pos();
		int x = np.x() - m_oLastPressPoint.x();
		int y = np.y() - m_oLastPressPoint.y();

		m_iWW = m_oBox->m_iWW + x;
		if (m_iWW < 2 * GRID) m_iWW = 2 * GRID;
		m_iWW = grid_int(m_iWW);

		m_iHH = m_oBox->m_iHH + y;
		if (m_iHH < 2 * GRID) m_iHH = 2 * GRID;
		m_iHH = grid_int(m_iHH);

		doc.setTextWidth(m_iWW - 2 * OFF);
		setRect(0, 0, m_iWW, m_iHH);
		m_oChain->setPos(m_iWW + 3, 0);

		m_oView->message(m_oView->trUtf8("%1 x %2").arg(QString::number(m_iWW), QString::number(m_iHH)), 1000);

		update();
		update_links();
	}
	else
	{
		QGraphicsRectItem::mouseMoveEvent(e);
	}
}

void box_item::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
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
		}
	}
	else
	{
		QGraphicsRectItem::mouseReleaseEvent(e);
	}
}

void box_item::update_data() {
	setPos(QPointF(m_oBox->m_iXX, m_oBox->m_iYY));
	if (m_oBox->m_iWW != m_iWW || m_oBox->m_iHH != m_iHH || doc.toPlainText() != m_oBox->m_sText)
	{
		update_size();
	}
	update();
}

void box_item::update_size() {
	m_iWW = m_oBox->m_iWW;
	m_iHH = m_oBox->m_iHH;

	doc.setHtml(QString("<div align='center'>%1</div>").arg(m_oBox->m_sText));
	doc.setTextWidth(m_iWW - 2 * OFF);

	setRect(0, 0, m_iWW, m_iHH);
	m_oChain->setPos(m_iWW + 3, 0);

	update_links();
}

void box_item::properties()
{
	bool ok = false;
	QString text = QInputDialog::getText(m_oView, m_oView->trUtf8("Diagram box properties"),
			m_oView->trUtf8("Text:"), QLineEdit::Normal, m_oBox->m_sText, &ok);
	if (ok && text != m_oBox->m_sText)
	{
		mem_edit_box *ed = new mem_edit_box(m_oView->m_oMediator, m_oView->m_iId, m_iId);
		ed->newText = text;

		QTextDocument doc;
		doc.setHtml(QString("<div align='center'>%1</div>").arg(text));
		doc.setTextWidth(m_oBox->m_iWW - 2 * OFF);
		ed->newHeight = GRID * (((int) (doc.size().height() + 2 * OFF + GRID - 1)) / GRID);
		if (ed->newHeight < m_oBox->m_iHH)
			ed->newHeight = m_oBox->m_iHH;

		ed->apply();
	}
}

QVariant box_item::itemChange(GraphicsItemChange i_oChange, const QVariant &i_oValue)
{
	if (scene())
	{
		if (i_oChange == ItemPositionChange)
		{
			QPointF np = i_oValue.toPointF();
			np.setX(((int) np.x() / GRID) * GRID);
			np.setY(((int) np.y() / GRID) * GRID);
			return np;
		}
		else if (i_oChange == ItemPositionHasChanged)
		{
			update_links();
		}
		else if (i_oChange == ItemSelectedHasChanged)
		{
			m_oChain->setVisible(isSelected());
		}
	}

	return QGraphicsItem::itemChange(i_oChange, i_oValue);
}

void box_item::update_links()
{
	// FIXME
	foreach (box_link* l_oLink, m_oView->m_oLinks)
	{
		l_oLink->update_pos();
	}
}

static int RATIO[] = {333, 500, 667, 0};

int box_item::choose_position(const QPointF& i_oP, int id)
{
	QRectF r = rect();
	QPointF l_o = pos() - i_oP + QPointF(r.width()/2, r.height()/2);
	double c_x = l_o.x() * r.height();
	double c_y = l_o.y() * r.width();

	int ret = 0;
	int best = 1<<30;
	int cand = 0;
	if (qAbs(c_x) > qAbs(c_y))
	{
		ret = (c_x > 0) ? data_link::WEST : data_link::EAST;
		for (int i=0; i < 10; ++i) {
			int k = RATIO[i];
			if (k == 0) break;
			int val = qAbs((k * r.height() / 1000.) - (i_oP.y() - pos().y()));

			if (val < best)
			{
				best = val;
				cand = k;
			}
		}
		ret += cand * MUL;
	}
	else
	{
		ret = (c_y > 0) ? data_link::NORTH : data_link::SOUTH;
		for (int i=0; i < 10; ++i) {
			int k = RATIO[i];
			if (k == 0) break;
			int val = qAbs((k * r.width() / 1000.) - (i_oP.x() - pos().x()));

			if (val < best)
			{
				best = val;
				cand = k;
			}
		}
		ret += cand * MUL;
	}
	return ret;
}

QPoint box_item::get_point(int i_oP)
{
	QRectF r = rect();
	int ratio = i_oP / MUL;

	if (ratio >= 1000 || ratio <= 0) ratio = 500;
	switch (i_oP & data_link::COORD) {
		case data_link::NORTH:
			return QPoint(r.x() + r.width() * ratio / 1000., r.y());
		case data_link::WEST:
			return QPoint(r.x(), r.y() + r.height() * ratio / 1000.);
		case data_link::SOUTH:
			return QPoint(r.x() + r.width() * ratio / 1000., r.y() + r.height());
		case data_link::EAST:
			return QPoint(r.x() + r.width(), r.y() + r.height() * ratio / 1000.);
	}
	Q_ASSERT(false);
	return QPoint(0, 0);
}

