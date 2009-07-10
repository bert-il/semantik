// Thomas Nagy 2007-2009 GPLV3

#include <QtDebug>
#include <QTreeWidget>
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "data_item.h"
#include "data_control.h"
#include "linear_view.h"
#include "con.h" 

linear_view::linear_view(QWidget *i_oParent, data_control *i_oControl) : QTreeWidget(i_oParent)
{
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);
	header()->hide();
	m_oControl = i_oControl;
	//connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
	//	this, SLOT(doubleClickHandler(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemSelectionChanged()),
		this, SLOT(selection_changed()));
	m_bLockSelect = false;
}

void linear_view::synchro_doc(const hash_params&i_o)
{
	int l_iCmd = i_o[data_commande].toInt();
	switch (l_iCmd)
	{
		case cmd_add_item:
			{
				QTreeWidgetItem *l_oItem = new QTreeWidgetItem(this);
				int l_iId = i_o[data_id].toInt();
				l_oItem->setText(0, m_oControl->m_oItems.value(l_iId)->m_sSummary);
				l_oItem->setData(0, Qt::UserRole, i_o[data_id]);
				//l_oItem->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
				addTopLevelItem(l_oItem);
				m_oItems[l_iId] = l_oItem;
			}
			break;
		case cmd_update_item:
			{
				if (i_o[data_orig].toInt() == view_linear) return;

				int l_iId = i_o[data_id].toInt();
				QTreeWidgetItem *l_oItem = m_oItems.value(l_iId);
				l_oItem->setText(0, m_oControl->m_oItems.value(l_iId)->m_sSummary);

				data_item *l_o = m_oControl->m_oItems.value(l_iId);
				l_oItem->setBackgroundColor(0, l_o->get_color_scheme().m_oInnerColor);
			}
			break;
		case cmd_select_item:
			{
				m_bLockSelect = true;

				if (i_o[data_orig].toInt() == view_linear)
				{
					m_bLockSelect = false;
					return;
				}

				int l_iId = i_o[data_id].toInt();

				QList<QTreeWidgetItem *> l_oLst = selectedItems();
				if (l_oLst.size()>1)
				{
					foreach (QTreeWidgetItem* l_oItem, l_oLst)
					{
						l_oItem->setSelected(false);
					}
				}
				else if (l_oLst.size()==1)
				{
					QTreeWidgetItem *l_oItem = l_oLst.at(0);
					int l_iIdOld = l_oItem->data(0, Qt::UserRole).toInt();
					if (l_iIdOld != l_iId)
					{
						l_oItem->setSelected(false);
						if (l_iId>0)
						{
							m_oItems.value(l_iId)->setSelected(true);
							m_oItems.value(l_iId)->setExpanded(true);
						}
					}
				}
				else
				{
					//TODO dead code above
					if (l_iId>NO_ITEM)
					{
						m_oItems.value(l_iId)->setSelected(true);
						m_oItems.value(l_iId)->setExpanded(true);
					}
				}
				m_bLockSelect = false;
			}
			break;
		case cmd_link:
			{
				QTreeWidgetItem *l_oItem1 = m_oItems.value(i_o[data_id].toInt());
				QTreeWidgetItem *l_oItem2 = m_oItems.value(i_o[data_parent].toInt());
				QTreeWidgetItem *l_oRet = takeTopLevelItem(indexOfTopLevelItem(l_oItem1));
				Q_ASSERT(l_oRet != NULL);
				l_oItem2->insertChild(l_oItem2->childCount(), l_oItem1);
				l_oItem2->setExpanded(true);

				data_item *l_o = m_oControl->m_oItems.value(i_o[data_id].toInt());
				l_oItem1->setBackgroundColor(0, l_o->get_color_scheme().m_oInnerColor);
			}
			break;
		case cmd_unlink:
			{
				QTreeWidgetItem *l_oItem1 = m_oItems.value(i_o[data_id1].toInt());
				QTreeWidgetItem *l_oItem2 = m_oItems.value(i_o[data_id2].toInt());
				if (l_oItem1->parent() == l_oItem2)
				{
					l_oItem2->takeChild(l_oItem2->indexOfChild(l_oItem1));
					l_oItem2->setExpanded(true);
					addTopLevelItem(l_oItem1);

					data_item *l_o = m_oControl->m_oItems.value(i_o[data_id1].toInt());
					l_oItem1->setBackgroundColor(0, l_o->get_color_scheme().m_oInnerColor);
				}
				else if (l_oItem2->parent() == l_oItem1)
				{
					l_oItem1->takeChild(l_oItem1->indexOfChild(l_oItem2));
					l_oItem1->setExpanded(true);
					addTopLevelItem(l_oItem2);

					data_item *l_o = m_oControl->m_oItems.value(i_o[data_id2].toInt());
					l_oItem2->setBackgroundColor(0, l_o->get_color_scheme().m_oInnerColor);
				}
				else
				{
					Q_ASSERT(1>1);
				}
			}
			break;
		case cmd_remove_item:
			{
				QTreeWidgetItem *l_oItem = m_oItems.value(i_o[data_id].toInt());

				QTreeWidgetItem *l_oChild = NULL;
				while ( (l_oChild = l_oItem->takeChild(0)) != NULL)
				{
					addTopLevelItem(l_oChild);
				}
				if (l_oItem->parent())
					l_oItem->parent()->takeChild(l_oItem->parent()->indexOfChild(l_oItem));
				else
					takeTopLevelItem(indexOfTopLevelItem(l_oItem));
				delete l_oItem;
			}
			break;
		case cmd_sort_item:
			{
				if (i_o[data_orig].toInt() == view_linear) return;

				int l_iId = i_o[data_id].toInt();

				QTreeWidgetItem *l_oItem = m_oItems.value(l_iId);
				int l_iCnt = 0;
				for (int i=0; i<m_oControl->m_oLinks.size(); i++)
				{
					QPoint l_oP = m_oControl->m_oLinks.at(i);
					if (l_oP.x() == l_iId)
					{
						QTreeWidgetItem *l_oItem2 = m_oItems.value(l_oP.y());
						int l_iIndex = l_oItem->indexOfChild(l_oItem2);
						if (l_iIndex != l_iCnt)
						{
							l_oItem->takeChild(l_iIndex);
							l_oItem->insertChild(l_iCnt, l_oItem2);
						}
						l_iCnt++;
					}
				}
			}
			break;
		default:
			break;
	}
}

#if 0
void linear_view::doubleClickHandler(QTreeWidgetItem* i_oWidget, int)
{
	if (i_oWidget)
	{
		QTreeWidgetItem *l_oItem = new QTreeWidgetItem(i_oWidget);
		l_oItem->setText(0, "hello");
	}
	else
	{
		QTreeWidgetItem *l_oItem = new QTreeWidgetItem(this);
		l_oItem->setText(0, "hello");
		addTopLevelItem(l_oItem);
	}
}
#endif

void linear_view::selection_changed()
{
	if (!m_bLockSelect)
	{
		QList<QTreeWidgetItem*> l_oItems = selectedItems();
		if (l_oItems.size())
		{
			QTreeWidgetItem *l_oItem = l_oItems.at(0);
			int l_iIdOld = l_oItem->data(0, Qt::UserRole).toInt();
			m_oControl->select_item(l_iIdOld, view_linear);
		}
	}
}

void linear_view::filter_slot(const QString & i_oS)
{
	foreach (QTreeWidgetItem *l_oItem, m_oItems.values())
	{
#if 0
		if (i_oS.length() < 1)
		{
			l_oItem->setHidden(false);
		}
#endif

		if (l_oItem->parent() != NULL) continue;
		filter_item(l_oItem, i_oS);
	}
}

bool linear_view::filter_item(QTreeWidgetItem * i_oItem, const QString & i_oS)
{
	bool l_bDisp = (i_oItem->text(0).indexOf(i_oS, 0, Qt::CaseInsensitive) >= 0);
	int l_iCount = i_oItem->childCount();

	for (int i=0; i<l_iCount; ++i)
	{
		QTreeWidgetItem *l_oChild = i_oItem->child(i);
		l_bDisp = filter_item(l_oChild, i_oS) || l_bDisp;
	}

	i_oItem->setHidden(!l_bDisp);
	return l_bDisp;
}

void linear_view::dropEvent(QDropEvent *i_oEv)
{
	if (i_oEv->source() == this)
	{
		QTreeWidgetItem *l_oItem = itemAt(i_oEv->pos());
		QTreeWidgetItem *l_oChild = selectedItems().at(0);
		Q_ASSERT(l_oChild != NULL);

		if (l_oChild)
		{
			int l_iId = l_oChild->data(0, Qt::UserRole).toInt();
			if (dropIndicatorPosition() == QAbstractItemView::OnItem)
			{
				if (m_oControl->parent_of(l_iId)) m_oControl->unlink_items(l_iId, m_oControl->parent_of(l_iId));
				if (l_oItem != NULL)
				{
					m_oControl->link_items(l_oItem->data(0, Qt::UserRole).toInt(), l_iId);
				}
			}
			else if (l_oItem != NULL)
			{
				int j = l_oItem->data(0, Qt::UserRole).toInt();
				int k = m_oControl->parent_of(j);

				// make certain they have the same parent now
				if (m_oControl->parent_of(l_iId) != k);
				{
					if (m_oControl->parent_of(l_iId)) m_oControl->unlink_items(l_iId, m_oControl->parent_of(l_iId));
					m_oControl->link_items(k, l_iId);
				}

				int z = 1; // offset for re-inserting the item
				if (dropIndicatorPosition() == QAbstractItemView::AboveItem)
				{
					z = 0;
				}

				int l = 0; // the index of the item drop
				for (int i=0; i<m_oControl->m_oLinks.size(); i++)
				{
					QPoint l_oP = m_oControl->m_oLinks.at(i);
					if (l_oP.x() == k) // same parent
					{
						if (l_oP.y() == j) // item found
						{
							m_oControl->sort_children(k, l_iId, l+z);
							break;
						}
						l++;
					}
				}
			}
			else
			{
				if (m_oControl->parent_of(l_iId)) m_oControl->unlink_items(l_iId, m_oControl->parent_of(l_iId));
			}
			m_oControl->select_item(l_iId);
		}
	}
	i_oEv->accept();
	i_oEv->setDropAction(Qt::CopyAction);
}

#include "linear_view.moc"

