// Thomas Nagy 2007-2011 GPLV3

#ifndef DATA_CONTROL_H
#define DATA_CONTROL_H

#include <QObject>
#include <QStack>
#include <QXmlDefaultHandler>
#include <QMutex>
#include<KUrl> 

#include "aux.h"
#include "con.h"
#include "mem_command.h"
#include "flag_scheme.h"
#include "color_scheme.h"

class QTimer;
class html_converter : public QXmlDefaultHandler
{
	public:
		html_converter();

		bool startElement(const QString&, const QString&, const QString&, const QXmlAttributes&);
		bool endElement(const QString&, const QString&, const QString&);
		bool characters(const QString &i_sStr);

		QString m_sBuf;
		QStringList m_oTotale;
};


// sem_model -> mediator

class data_item;
class bind_node;
class sem_model: public QObject
{
	Q_OBJECT

	signals:
		//void rectangle_changed(int i);

		void synchro(const hash_params&);

		void sig_message(const QString&, int);
		void dirty(bool);
		void update_title();

		void sig_add_item(int id);
		void sig_delete_item(int id);
		void sig_link_items(int id1, int id2);
		void sig_unlink_items(int id1, int id2);
		void sync_flags();
		void sync_colors();
		void sig_select(const QList<int>& unsel, const QList<int>& sel);
		void sig_move(const QList<int>&sel, const QList<QPointF>&pos);
		void sig_repaint(int id);
		void sig_flag(int id);
		void sig_edit(int id);
		void sig_text(int id);
		void sig_datatype(int id);
		void sig_preview();
		void sig_vars(int id);
		void sig_pic(int id);
		void sig_table(int id);
		void sig_sort(int id);
		void sig_save_data();
		void sig_open_map();

		void enable_undo(bool, bool);

	public:
		sem_model(QObject *i_oParent);
		~sem_model();

		int seq();

		void undo_purge();

		QString m_sTempDir;
		QString m_sOutDir;
		QString m_sOutProject;
		QString m_sOutTemplate;

		void set_dirty(bool b = true);
		bool m_bDirty;

		void init_temp_dir();
		void clean_temp_dir();

		bool save_file(QString);
		bool open_file(const QString&);
		void purge_document();
		bool read_xml_file(const QString &l_oBa);

		void init_colors();
		void init_flags();

		// first parameter is the parent, second parameter is the wanted id
		// and the third parameter is for copying the parents data
		bool link_items(int id1, int id2);

		int num_children(int i_iParent);
		int m_iConnType;
		int m_iReorgType;
		void select_root_item(int);
		QList<int> all_roots();
		int root_of(int i_iId);
		void next_root();
		void prev_root();
		double m_dTriSize;

		QColor m_oColor;
		QString m_sHints;

		QStack<mem_command*> m_oUndoStack;
		QStack<mem_command*> m_oRedoStack;

		void notify_add_item(int id);
		void notify_delete_item(int id);
		void notify_link_items(int id1, int id2);
		void notify_unlink_items(int id1, int id2);
		void notify_select(const QList<int>& unsel, const QList<int>& sel);
		void notify_move(const QList<int>&sel, const QList<QPointF>&pos);
		void notify_repaint(int id);
		void notify_edit(int id);
		void notify_text(int id);
		void notify_vars(int id);
		void notify_flag(int id);
		void notify_pic(int id);
		void notify_datatype(int id);
		void notify_table(int id);
		void notify_sort(int id);

	public:

		KUrl m_oCurrentUrl;

		QHash<int, data_item*> m_oItems;
		QList<QPoint> m_oLinks;
		QList<int> m_oImgs;

		QList<color_scheme> m_oColorSchemes;
		QList<flag_scheme*> m_oFlagSchemes;

		int next_seq();

		void generate_docs(const QString &i_oFile, const QString &i_sName, const QString &i_sLocation);

		int m_iTimerValue;
		int parent_of(int i_iId);
		int size_of(int i_iId);
		bind_node* create_tree(int);
		int choose_root();

		void init_timer();
		void destroy_timer();

		QString doc_to_xml();
		QString m_sLastSaved;

		data_item* operator+(const int x);

		void select_item_keyboard(int, int);
		void private_select_item(int i_oId);

		void change_data(int i_iId, int i_iType);

		void check_undo(bool); // check if the undo/redo actions can be enabled
		int itemSelected();

		QMutex m_oLock;

	private:
		int num_seq;
		QTimer *m_oTimer;

	public slots:
		//void do_reorganize();
		void slot_autosave();
		void slot_undo();
		void slot_redo();
};

QIcon fetch_icon(const QString& i_sName, int i_iSize=32);

#endif

