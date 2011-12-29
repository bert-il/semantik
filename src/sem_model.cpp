// Thomas Nagy 2007-2011 GPLV3

#include <sys/time.h>

%: include  <Python.h> 
 #include <QFile>
#include<QTimer>
 #include <QtDebug>
  %:	include<QCoreApplication>
  #include <QXmlDefaultHandler>
#include<KConfigGroup>
%:include\
<KGlobal>
#include<KConfig>
 #include <QProcess>   
  #include <QDir>
/*
 */ #include <sys/ioctl.h>
  #include <termios.h>
#include <sonnet/dialog.h>
/*
  */ %: include "con.h"  
        #include <stdlib.h> 
 #include "sembind.h"  	
  #include "data_item.h"	
  %: include "sem_model.h" 
#include <KMessageBox>

class semantik_reader : public QXmlDefaultHandler
{
	public:
		semantik_reader(sem_model*);

	private:
		QString m_sBuf;
		int m_iVersion;
		sem_model *m_oControl;
		int m_iId;

	public:
		bool startElement(const QString&, const QString&, const QString&, const QXmlAttributes&);
		bool endElement(const QString&, const QString&, const QString&);
		bool characters(const QString &i_sStr);

};

semantik_reader::semantik_reader(sem_model *i_oControl)
{
	m_oControl = i_oControl;
}

bool semantik_reader::startElement(const QString&, const QString&, const QString& i_sName, const QXmlAttributes& i_oAttrs)
{
	if (i_sName == notr("info"))
	{
		//m_oControl->m_sName = i_oAttrs.value("name");
		//m_oControl->m_sFirstName = i_oAttrs.value("fname");
		//m_oControl->m_sOrganization = i_oAttrs.value("organization");
		//m_oControl->m_sEmail = i_oAttrs.value("email");
		m_oControl->m_sHints = i_oAttrs.value(notr("hints"));

		if (i_oAttrs.value(notr("location")).size()) m_oControl->m_sOutDir = i_oAttrs.value(notr("location"));
		if (i_oAttrs.value(notr("dir")).size()) m_oControl->m_sOutProject = i_oAttrs.value(notr("dir"));
		if (i_oAttrs.value(notr("output")).size()) m_oControl->m_sOutTemplate = i_oAttrs.value(notr("output"));
	}
	else if (i_sName == notr("link"))
	{
		int a = i_oAttrs.value(notr("p")).toInt();
		int b = i_oAttrs.value(notr("v")).toInt();
		if (!m_oControl->m_oItems.contains(a)) return false;
		if (!m_oControl->m_oItems.contains(b)) return false;
		m_oControl->m_oLinks.append(QPoint(a, b));
	}
	else if (i_sName == notr("tblsettings"))
	{
		data_item *l_oItem = m_oControl->m_oItems.value(m_iId);
		l_oItem->m_iNumRows = i_oAttrs.value(notr("rows")).toInt();
		l_oItem->m_iNumCols = i_oAttrs.value(notr("cols")).toInt();
	}
	else if (i_sName == notr("itembox"))
	{
		data_item *l_oItem = m_oControl->m_oItems.value(m_iId);
		int bid = i_oAttrs.value(notr("id")).toInt();
		data_box *box = new data_box(bid);
		l_oItem->m_oBoxes[bid] = box;
		box->m_sText = i_oAttrs.value(notr("text"));
		box->m_iXX = i_oAttrs.value(notr("x")).toFloat();
		box->m_iYY = i_oAttrs.value(notr("y")).toFloat();
		box->m_iWW = i_oAttrs.value(notr("w")).toFloat();
		box->m_iHH = i_oAttrs.value(notr("h")).toFloat();
	}
	else if (i_sName == notr("tbl"))
	{
		data_item *l_oItem = m_oControl->m_oItems.value(m_iId);
		int row = i_oAttrs.value(notr("row")).toInt();
		int col = i_oAttrs.value(notr("col")).toInt();
		QPair<int, int> p(row, col);
		l_oItem->m_oTableData[p] = i_oAttrs.value(notr("text"));
	}
	else if (i_sName == notr("item"))
	{
		m_iId = i_oAttrs.value(notr("id")).toInt();
		if (!m_iId) return false;

		data_item *l_oItem = new data_item(m_oControl, m_iId);
		m_oControl->m_oItems[m_iId] = l_oItem;

		l_oItem->m_sSummary = i_oAttrs.value(notr("summary"));
		l_oItem->m_sText = i_oAttrs.value(notr("text"));
		l_oItem->m_iTextLength = i_oAttrs.value(notr("len")).toInt();
		l_oItem->m_sComment = i_oAttrs.value(notr("comment"));

		l_oItem->m_sPicLocation = i_oAttrs.value(notr("pic_location"));
		l_oItem->m_sPicCaption = i_oAttrs.value(notr("pic_caption"));
		l_oItem->m_sPicComment = i_oAttrs.value(notr("pic_comment"));

		l_oItem->m_iDataType = i_oAttrs.value(notr("data")).toInt();
		if (l_oItem->m_iDataType == 0) l_oItem->m_iDataType = VIEW_TEXT;

		l_oItem->m_sHints = i_oAttrs.value(notr("hints"));

		l_oItem->m_iNumRows = i_oAttrs.value(notr("tbl_rows")).toInt();
		l_oItem->m_iNumCols = i_oAttrs.value(notr("tbl_cols")).toInt();

		l_oItem->m_sDiag = i_oAttrs.value(notr("dg"));

		l_oItem->m_iXX = i_oAttrs.value(notr("c2")).toDouble();
		l_oItem->m_iYY = i_oAttrs.value(notr("c1")).toDouble();

		l_oItem->m_iColor = i_oAttrs.value(notr("color")).toInt();
		l_oItem->m_oCustom.m_sName = i_oAttrs.value(notr("custom_name"));
		l_oItem->m_oCustom.m_oInnerColor = QColor(i_oAttrs.value(notr("custom_inner")));
		l_oItem->m_oCustom.m_oBorderColor = QColor(i_oAttrs.value(notr("custom_border")));
		l_oItem->m_oCustom.m_oTextColor = QColor(i_oAttrs.value(notr("custom_text")));
	}
	else if (i_sName == notr("color_schemes"))
	{
		m_oControl->m_oColorSchemes.clear();
	}
	else if (i_sName == notr("color_scheme"))
	{
		color_scheme l_o;
		l_o.m_sName = i_oAttrs.value(notr("name"));
		l_o.m_oInnerColor = i_oAttrs.value(notr("inner"));
		l_o.m_oBorderColor = i_oAttrs.value(notr("border"));
		l_o.m_oTextColor = i_oAttrs.value(notr("text"));
		m_oControl->m_oColorSchemes.push_back(l_o);
	}
	else if (i_sName == notr("flag"))
	{
		data_item *l_oItem = m_oControl->m_oItems.value(m_iId);
		l_oItem->m_oFlags.push_back(i_oAttrs.value(notr("id")));
	}
	return true;
}

bool semantik_reader::endElement(const QString&, const QString&, const QString& i_sName)
{
	return true;
}

bool semantik_reader::characters(const QString &i_s)
{
    m_sBuf += i_s;
    return true;
}

sem_model::~sem_model()
{
	destroy_timer();
	clean_temp_dir();
	while (!m_oFlagSchemes.empty())
	{
		delete m_oFlagSchemes.takeFirst();
	}
}

void sem_model::init_timer()
{
	destroy_timer();
	if (m_iTimerValue<1) return;
	m_oTimer = new QTimer(this);
	m_oTimer->setInterval(m_iTimerValue * 60000);
	connect(m_oTimer, SIGNAL(timeout()), this, SLOT(slot_autosave()));
	m_oTimer->start();
}

void sem_model::destroy_timer()
{
	if (m_oTimer) m_oTimer->stop();
	delete m_oTimer;
	m_oTimer = NULL;
}

void sem_model::slot_autosave()
{
	// autosave for the last used save name
	qDebug()<<"autosave"<<m_sLastSaved;
	if (m_sLastSaved.length() > 1)
	{
		QString o = trUtf8("Automatic save failed for file: %1").arg(m_sLastSaved);
		if (save_file(m_sLastSaved))
		{
			o = trUtf8("%1 saved automatically").arg(m_sLastSaved);
		}
		emit sig_message(o, 5000);
	}
}

void sem_model::init_colors()
{
	m_oColorSchemes.clear();

	QStringList l_oS;
	//l_oS<<"#FFFCD5"<<"#ABFBC7"<<"#FDE1E1"<<"#D2F1FF"<<"#EDDFFF"<<"#FFFFFF";
	l_oS<<"#fffe8d"<<"#91ffab"<<"#9bfffe"<<"#b8bbff"<<"#e0aaff"<<"#ffa6a6"<<"#ffd8a6"<<"#FFFFFF"<<"#d3d7cf";

	int i = 0;
	foreach (QString l_s, l_oS)
	{
		color_scheme l_o;
		l_o.m_oInnerColor = QColor(l_s);
		l_o.m_sName = trUtf8("Color %1").arg(QString::number(i));
		m_oColorSchemes.push_back(l_o);
		++i;
	}

	KConfigGroup l_oSettings(KGlobal::config(), notr("General Options"));
        m_iConnType = l_oSettings.readEntry(notr("conn"), 0);
        m_iReorgType = l_oSettings.readEntry(notr("reorg"), 0);
	m_dTriSize = l_oSettings.readEntry(notr("trisize"), (double) 4.5);
	m_iTimerValue = l_oSettings.readEntry(notr("auto"), 0);

	init_timer();

	emit sync_colors();
}

void sem_model::init_flags()
{
	while (!m_oFlagSchemes.empty())
	{
		delete m_oFlagSchemes.takeFirst();
	}

	QStringList l_oLst = QString(notr("flag_delay flag_idea flag_look flag_lunch flag_money flag_ok flag_people flag_phone flag_star flag_stop flag_talk flag_target flag_time flag_tune flag_unknown flag_write")).split(" ");
	foreach (QString l_s, l_oLst)
	{
		m_oFlagSchemes.push_back(new flag_scheme(this, l_s, l_s));
	}

	emit sync_flags();
}

void sem_model::init_temp_dir()
{
	char sfn[16] = "";
	strcpy(sfn, "/tmp/sem.XXXXXX");
	char * l_oRet = mkdtemp(sfn);
	m_sTempDir = l_oRet;

	Q_ASSERT(l_oRet!=NULL);
	Q_ASSERT(QFile::exists(m_sTempDir));
}

void sem_model::clean_temp_dir()
{
	//qDebug()<<"sem_model::clean_temp_dir";
	QProcess l_oP;
	QStringList l_oArgs;
	l_oArgs<<notr("-rf")<<m_sTempDir;
	l_oP.start(notr("/bin/rm"), l_oArgs);
	l_oP.waitForFinished();
	//qDebug()<<"rmdir"<<m_sTempDir;
}

/* here magic happens */
QByteArray new_header(const QString & i_oName, int i_iLen)
{
	QByteArray l_oBuf;
	l_oBuf.fill(0x0, 512);

	QByteArray l_oArr;
	l_oArr.append(i_oName);
	l_oBuf = l_oBuf.replace(0, l_oArr.length(), l_oArr);

	// mode and permission
	QByteArray l_oBuf1("0000600");
	l_oBuf.replace(100, l_oBuf1.length(), l_oBuf1);

	QByteArray l_oBuf2("0000000");
	l_oBuf.replace(108, l_oBuf2.length(), l_oBuf2);

	QByteArray l_oBuf3("0000000");
	l_oBuf.replace(116, l_oBuf3.length(), l_oBuf3);


	// write the size
	QByteArray l_oSize;
	l_oSize.setNum(i_iLen, 8);
	l_oSize = l_oSize.rightJustified(11, '0');
	l_oBuf = l_oBuf.replace(124, l_oSize.length(), l_oSize);

	// last modification time
	QByteArray l_oTime;
	qlonglong r = 1170500000L;
	l_oTime.setNum(r, 8);
	l_oTime = l_oTime.rightJustified(11, '0');
	l_oBuf = l_oBuf.replace(136, l_oTime.length(), l_oTime);

	// checksum
	QByteArray l_oCheckSum;
	l_oCheckSum.fill(' ', 6);
	l_oBuf = l_oBuf.replace(148, l_oCheckSum.length(), l_oCheckSum);

	// file type - leave null for now
	QByteArray l_oFileType;
	l_oFileType.fill(' ', 1);
	l_oBuf = l_oBuf.replace(155, l_oFileType.length(), l_oFileType);
	l_oFileType.fill('0', 1);
	l_oBuf = l_oBuf.replace(156, l_oFileType.length(), l_oFileType);


	// compute the checksum
	int l_iChk = 32;
	for (int i = 0; i<512; ++i) l_iChk += l_oBuf[i];
	l_oCheckSum.setNum(l_iChk, 8);
	l_oCheckSum = l_oCheckSum.rightJustified(6, '0');
	l_oBuf = l_oBuf.replace(148, l_oCheckSum.length(), l_oCheckSum);

	return l_oBuf;
}

#if 0
void sem_model::do_reorganize()
{
	hash_params l_oCmd;
	l_oCmd.insert(data_commande, QVariant(cmd_save_data));
	emit synchro(l_oCmd);

	foreach (int l_iVal, m_oItems.keys())
	{
		data_item *l_oData = m_oItems.value(l_iVal);
		//add_ox(l_iVal, (double) l_oData->m_iXX + l_oData->m_iWW/2,
		//	(double) l_oData->m_iYY + l_oData->m_iHH/2,
		//	(double) l_oData->m_iWW/2, (double) l_oData->m_iHH/2);
	}
	for (int i=0; i<m_oLinks.size(); i++)
	{
		QPoint l_oP = m_oLinks.at(i);
		//add_cstr(l_oP.x(), l_oP.y());
	}

        //struct timeval start_time;
        //gettimeofday(&start_time, NULL);
        //int m_iUSEC_start = start_time.tv_usec;
        //int m_iSEC_start = start_time.tv_sec;

	// iterate
	//iterate_pb(m_iReorgType);

        //struct timeval stop_time;
        //gettimeofday(&stop_time, NULL);
        //int m_iUSEC_stop = stop_time.tv_usec;
        //int m_iSEC_stop = stop_time.tv_sec;

	//int l_iSectime = m_iSEC_stop  - m_iSEC_start;
        //int l_iUsectime = m_iUSEC_stop - m_iUSEC_start;
        //double l_dTotaltime = 0.;

        //if (l_iSectime < 0) l_iUsectime = - l_iUsectime;
        //l_dTotaltime = (1000000.0*(double)l_iSectime + (double)l_iUsectime)/1000000.0;
	//qDebug()<<l_dTotaltime;

	/*foreach (int l_iVal, m_oItems.keys())
	{
		data_item *l_oData = m_oItems.value(l_iVal);
		l_oData->m_iXX = get_x(l_iVal) - l_oData->m_iWW/2;
		l_oData->m_iYY = get_y(l_iVal) - l_oData->m_iHH/2;
		update_item(l_iVal);
	}
	cleanup_pb();

	hash_params l_oCmd2;
	l_oCmd2.insert(data_commande, QVariant(cmd_open_map));
	emit synchro(l_oCmd2);*/
}
#endif

QString sem_model::doc_to_xml()
{
	QStringList l_oS;

	l_oS<<notr("<?xml version=\"1.0\" encoding=\"utf8\"?>\n");
	l_oS<<notr("<semantik version=\"1\">\n");
	l_oS<<notr("\t<info");
	//l_oS<<QString(" name=\"%1\"").arg(bind_node::protectXML(m_sName));
	//l_oS<<QString(" fname=\"%1\"").arg(bind_node::protectXML(m_sFirstName));
	//l_oS<<QString(" organization=\"%1\"").arg(bind_node::protectXML(m_sOrganization));
	//l_oS<<QString(" email=\"%1\"").arg(bind_node::protectXML(m_sEmail));
	l_oS<<notr(" location=\"%1\"").arg(bind_node::protectXML(m_sOutDir));
	l_oS<<notr(" dir=\"%1\"").arg(bind_node::protectXML(m_sOutProject));
	l_oS<<notr(" output=\"%1\"").arg(bind_node::protectXML(m_sOutTemplate));
	l_oS<<notr(" hints=\"%1\"").arg(bind_node::protectXML(m_sHints));
	l_oS<<notr("/>\n");

	l_oS<<notr("<color_schemes>\n");
	foreach (color_scheme l_o, m_oColorSchemes)
	{
		l_oS<<notr("<color_scheme name=\"%1\" inner=\"%2\" border=\"%3\" text=\"%4\"/>\n").arg(
			l_o.m_sName,
			l_o.m_oInnerColor.name(),
			l_o.m_oBorderColor.name(),
			l_o.m_oTextColor.name()
			);
	}
	l_oS<<notr("</color_schemes>\n");


	QList<data_item*> l_oLst = m_oItems.values();
	for (int i=0; i<l_oLst.size(); i++)
	{
		data_item* l_oItem = l_oLst.at(i);
		l_oS<<notr("<item id=\"%1\"").arg(QString::number(l_oItem->m_iId));
		l_oS<<notr(" summary=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sSummary));
		l_oS<<notr(" text=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sText));
		l_oS<<notr(" len=\"%1\"").arg(QString::number(l_oItem->m_iTextLength));
		l_oS<<notr(" comment=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sComment));

		l_oS<<notr(" pic_location=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sPicLocation));
		l_oS<<notr(" pic_caption=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sPicCaption));
		l_oS<<notr(" pic_comment=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sPicComment));

		l_oS<<notr(" tbl_rows=\"%1\"").arg(QString::number(l_oItem->m_iNumRows));
		l_oS<<notr(" tbl_cols=\"%1\"").arg(QString::number(l_oItem->m_iNumCols));

		l_oS<<notr(" dg=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sDiag));

		l_oS<<notr(" hints=\"%1\"").arg(bind_node::protectXML(l_oItem->m_sHints));
		l_oS<<notr(" data=\"%1\"").arg(QString::number(l_oItem->m_iDataType));

		l_oS<<notr(" c1=\"%1\"").arg(QString::number(l_oItem->m_iYY));
		l_oS<<notr(" c2=\"%1\"").arg(QString::number(l_oItem->m_iXX));

		l_oS<<notr(" color=\"%1\"").arg(QString::number(l_oItem->m_iColor));

		color_scheme l_o = l_oItem->m_oCustom;

		l_oS<<notr(" custom_name=\"%1\"").arg(l_o.m_sName);
		l_oS<<notr(" custom_border=\"%1\"").arg(l_o.m_oBorderColor.name());
		l_oS<<notr(" custom_inner=\"%1\"").arg(l_o.m_oInnerColor.name());
		l_oS<<notr(" custom_text=\"%1\"").arg(l_o.m_oTextColor.name());


		l_oS<<notr(">\n");

		l_oS<<notr("<tblsettings rows=\"%1\" cols=\"%2\"/>\n").arg(
			QString::number(l_oItem->m_iNumRows), QString::number(l_oItem->m_iNumCols));


		QPair<int, int> p;
		foreach (p, l_oItem->m_oTableData.keys())
		{
			l_oS<<notr("<tbl");
			l_oS<<notr(" row=\"%1\"").arg(QString::number(p.first));
			l_oS<<notr(" col=\"%1\"").arg(QString::number(p.second));
			l_oS<<notr(" text=\"%1\"").arg(bind_node::protectXML(l_oItem->m_oTableData[p]));
			l_oS<<notr("/>\n");
		}

		foreach (QString l_s, l_oItem->m_oFlags)
		{
			l_oS<<notr("<flag id=\"%1\"/>\n").arg(bind_node::protectXML(l_s));
		}


		foreach (data_box *box, l_oItem->m_oBoxes)
		{
			l_oS<<notr("<itembox id=\"%1\" text=\"%2\" x=\"%3\" y=\"%4\" w=\"%5\" h=\"%6\">\n").arg(
				QString::number(box->m_iId),
				bind_node::protectXML(box->m_sText),
				QString::number(box->m_iXX),
				QString::number(box->m_iYY),
				QString::number(box->m_iWW),
				QString::number(box->m_iHH)
			);
			l_oS<<notr("</itembox>\n");
		}

		// TODO same thing for the links
		
		l_oS<<notr("</item>\n");
	}

	for (int i=0; i<m_oLinks.size(); i++)
	{
		QPoint l_oP = m_oLinks.at(i);
		l_oS<<notr("<link p=\"%1\" v=\"%2\"/>\n").arg(l_oP.x()).arg(l_oP.y());
	}

	l_oS<<notr("</semantik>\n");

	return l_oS.join("");
}

bool sem_model::save_file(QString i_sUrl)
{
	Q_ASSERT(i_sUrl.endsWith(".sem"));

	QFile l_o2(QString(TEMPLATE_DIR)+notr("/semantik.sem.py"));
	if (!l_o2.open(QIODevice::ReadOnly))
	{
		emit sig_message(trUtf8("File saving: missing file %1").arg(l_o2.fileName()), 5000);
		return false;
	}

	QByteArray l_oBa = l_o2.readAll();
	l_o2.close();

	bind_node::set_var(notr("temp_dir"), m_sTempDir);
	bind_node::set_var(notr("outfile"), i_sUrl);
	bind_node::set_var(notr("fulldoc"), doc_to_xml());
	bind_node::set_var(notr("hints"), m_sHints);
	bind_node::set_var(notr("namet"), "");
	bind_node::set_var(notr("outdir"), "");

	if (!init_py())
	{
		emit sig_message(trUtf8("Missing bindings for opening files"), 5000);
		return false;
	}
	PyRun_SimpleString(l_oBa.constData());

	m_sLastSaved = i_sUrl;

	return true;
}

void sem_model::purge_document()
{
	while (!m_oUndoStack.isEmpty())
		delete m_oUndoStack.pop();
	while (!m_oRedoStack.isEmpty())
		delete m_oRedoStack.pop();
	mem_delete* del = new mem_delete(this);
	del->init(m_oItems.keys());
	del->apply();
}

void sem_model::undo_purge() {
	m_oItems.clear();
	m_oLinks.clear();
	while (!m_oUndoStack.isEmpty()) {
		mem_command* c = m_oUndoStack.pop();
		c->undo();
	}
	while (!m_oRedoStack.isEmpty())
		delete m_oRedoStack.pop();
}

void sem_model::check_undo(bool enable) {
	if (!enable) {
		emit enable_undo(false, false);
	} else {
		emit enable_undo(!m_oUndoStack.isEmpty(), !m_oRedoStack.isEmpty());
	}
}

bool sem_model::open_file(const QString& i_sUrl)
{
	purge_document();

	QFile l_o2(QString(FILTER_DIR)+"/main.py");
	if (!l_o2.open(QIODevice::ReadOnly))
	{
		KMessageBox::sorry(NULL, trUtf8("Missing filter file %1 for opening files").arg(l_o2.fileName()), trUtf8("Broken installation"));
		undo_purge();
		return false;
	}
	QByteArray l_oBa = l_o2.readAll();
	l_o2.close();

	bind_node::set_var(notr("temp_dir"), m_sTempDir);
	bind_node::set_var(notr("pname"), i_sUrl);
	bind_node::set_var(notr("hints"), m_sHints);
	bind_node::set_var(notr("namet"), i_sUrl);
	bind_node::set_var(notr("outdir"), "");

	if (!init_py())
	{
		KMessageBox::sorry(NULL, trUtf8("Missing python bindings for opening files"), trUtf8("Broken installation"));
		undo_purge();
		return false;
	}
	PyRun_SimpleString(l_oBa.constData());

	delete bind_node::_root;
	bind_node::_root = NULL;

	//qDebug()<<"full text "<<bind_node::fulldoc()<<endl;

	bool l_bResult = read_xml_file(bind_node::get_var(notr("fulldoc")));
	if (!l_bResult) {
		undo_purge();
		KMessageBox::sorry(NULL, trUtf8("Could not load the document %1").arg(bind_node::get_var(notr("fulldoc"))), trUtf8("Broken document"));
		return false;
	}

	QDir l_oDir(m_sTempDir);
	QFileInfoList l_oLst = l_oDir.entryInfoList();

	// this is for the pictures
	foreach (QFileInfo l_oInfo, l_oLst)
	{
		QString l_sName = l_oInfo.fileName();
		if (!l_sName.contains(notr("pic-"))) continue;
		l_sName = l_sName.section(QRegExp(notr("[.-]")), 1, 1);

		int l_iVal = l_sName.toInt();
		data_item *l_oData = m_oItems.value(l_iVal);

		QString l_sNew = l_oInfo.absoluteFilePath();
		if (l_oData)
		l_oData->load_from_path(l_sNew);
	}

	// now update all items created
	foreach (int i, m_oItems.keys()) {
		notify_add_item(i);
	}
	foreach (QPoint p, m_oLinks) {
		notify_link_items(p.x(), p.y());
	}


	QList<int> lst;
	mem_sel *sel = new mem_sel(this);
	sel->sel = lst;
	sel->apply();


	// now tell all the views that a new map was loaded
	emit sig_open_map();
	emit sync_colors();

	QRegExp r(notr(".kdi$"));
	QString s(notr(".sem"));

	m_sLastSaved = i_sUrl;
	m_sLastSaved.replace(r, s);

	sel = new mem_sel(this);
	sel->sel = lst;
	sel->apply();

	set_dirty(false);
	return true;
}

bool sem_model::read_xml_file(const QString &l_oBa)
{
	semantik_reader l_oHandler(this);
	QXmlInputSource l_oSource;
	l_oSource.setData(l_oBa);
	QXmlSimpleReader l_oReader;
	l_oReader.setContentHandler(&l_oHandler);

	if (!l_oReader.parse(l_oSource))
	{
		qDebug()<<"parse error";
		return false;
	}

	return true;
}

bool sem_model::link_items(int i_iParent, int i_iChild)
{
	Q_ASSERT(m_oItems.contains(i_iParent) && m_oItems.contains(i_iChild));

	if (i_iParent == i_iChild) return false;

	// one parent
	for (int i=0; i<m_oLinks.size(); i++)
	{
		QPoint l_oP = m_oLinks.at(i);
		if (l_oP.y() == i_iChild)
			return false;
	}

	// cycles
	int l_iIdChild = i_iParent;
	while (l_iIdChild > NO_ITEM)
	{
		int l_iNew = NO_ITEM;
		for (int i=0; i<m_oLinks.size(); i++)
		{
			QPoint l_oP = m_oLinks.at(i);
			if (l_oP.y() == l_iIdChild)
			{
				if (l_oP.x() == i_iChild) return false;
				l_iNew = l_oP.x();
				break;
			}
		}
		l_iIdChild = l_iNew;
	}

	mem_link *lnk = new mem_link(this);
	lnk->parent = i_iParent;
	lnk->child = i_iChild;
	lnk->apply();

	return true;
}

QList<int> sem_model::all_roots()
{
	QList<int> l_o = QList<int> ();
	foreach (int l_iVal, m_oItems.keys())
	{
		for (int i=0; i<m_oLinks.size(); i++)
		{
			QPoint l_oP = m_oLinks.at(i);
			if (l_oP.y() == l_iVal) goto end;
		}
		l_o.push_back(l_iVal);
		end:;
	}
	qSort(l_o);
	return l_o;
}

int sem_model::root_of(int i_iId)
{
	if (i_iId==NO_ITEM) return NO_ITEM;
	for (int i=0; i<m_oLinks.size(); i++)
	{
		QPoint l_oP = m_oLinks.at(i);
		if (l_oP.y() == i_iId) return root_of(l_oP.x());
	}
	return i_iId;
}

int sem_model::itemSelected() {
	foreach (int l_iVal, m_oItems.keys())
	{
		if (m_oItems[l_iVal]->m_bSelected)
			return l_iVal;
	}
	return NO_ITEM;
}

void sem_model::next_root()
{
	QList<int> l_o = all_roots();
	if (l_o.size() == 0) return;

	int l_i = itemSelected();
	if (l_i == NO_ITEM && !l_o.empty())
	{
		private_select_item(l_o[0]);
	}

	for (int i=0; i<l_o.size(); i++)
	{
		if (l_o[i] == l_i)
		{
			if (i==l_o.size()-1) private_select_item(l_o[0]);
			else private_select_item(l_o[i+1]);
			return;
		}
	}
}

void sem_model::prev_root()
{
	QList<int> l_o = all_roots();

	int l_i = itemSelected();
	if (l_i == NO_ITEM && !l_o.empty())
	{
		private_select_item(l_o[0]);
	}

	for (int i=0; i<l_o.size(); i++)
	{
		if (l_o[i] == l_i)
		{
			if (i==0) private_select_item(l_o[l_o.size()-1]);
			else private_select_item(l_o[i-1]);
			return;
		}
	}
}

void sem_model::select_root_item(int i_iId)
{
	if (i_iId == NO_ITEM)
	{
		QList<int> l_o = all_roots();
		if (l_o.size() > 0) private_select_item(l_o[0]);
	}
	else
	{
		int l_iId = root_of(i_iId);
		private_select_item(l_iId);
	}
}

void sem_model::select_item_keyboard(int l_iId, int l_iDirection)
{
	if (l_iId == NO_ITEM)
	{
		select_root_item(NO_ITEM); return;
	}
	if (!m_oLinks.size()) return;

	switch (l_iDirection)
	{
		case 1: //left
			{
				int l_iParent = NO_ITEM;
				int i = 0;

				while (i<m_oLinks.size())
				{
					QPoint l_oP = m_oLinks.at(i);
					if (l_oP.y() == l_iId)
					{
						l_iParent = l_oP.x();
						break;
					}
					++i;
				}
				if (l_iParent <= NO_ITEM) return;

				while (true)
				{
					--i;
					if (i<0) i=m_oLinks.size()-1;

					QPoint l_oP = m_oLinks.at(i);
					if (l_oP.x() == l_iParent)
					{
						private_select_item(l_oP.y());
						break;
					}
				}
			}
			break;
		case 2: //right
			{
				int l_iParent = NO_ITEM;
				int i = 0;

				while (i<m_oLinks.size())
				{
					QPoint l_oP = m_oLinks.at(i);
					if (l_oP.y() == l_iId)
					{
						l_iParent = l_oP.x();
						break;
					}
					++i;
				}
				if (l_iParent <= NO_ITEM) return;

				while (true)
				{
					++i;
					if (i >= m_oLinks.size()) i=0;

					QPoint l_oP = m_oLinks.at(i);
					if (l_oP.x() == l_iParent)
					{
						private_select_item(l_oP.y());
						break;
					}
				}
			}
			break;
		case 3: //top
			{
				for (int i=0; i<m_oLinks.size(); i++)
				{
					QPoint l_oP = m_oLinks.at(i);
					if (l_oP.y() == l_iId)
					{
						data_item *l_oItem = m_oItems.value(l_oP.x());
						// update the cache
						l_oItem->m_iDown = l_iId;
						private_select_item(l_oP.x());
						return;
					}
				}
			}
			break;
		case 4: //bottom
			{
				// first try the cache
				data_item *l_oItem = m_oItems.value(l_iId);
				Q_ASSERT(l_oItem != NULL);
				int l_iDown = l_oItem->m_iDown;

				for (int i=0; i<m_oLinks.size(); i++)
				{
					QPoint l_oP = m_oLinks.at(i);
					if (l_oP.x() == l_iId && l_oP.y() == l_iDown)
					{
						private_select_item(l_oP.y());
						return;
					}
				}

				// it did not work, take the first item in the list
				for (int i=0; i<m_oLinks.size(); i++)
				{
					QPoint l_oP = m_oLinks.at(i);
					if (l_oP.x() == l_iId)
					{
						private_select_item(l_oP.y());
						return;
					}
				}
			}
			break;
		default:
			break;
	};
}

int sem_model::next_seq()
{
	do {
		++num_seq;
	} while (m_oItems.contains(num_seq));
	return num_seq;
}

void sem_model::set_dirty(bool b)
{
	if (b != m_bDirty)
	{
		m_bDirty = b;
		//if (b) emit sig_message(trUtf8("dirty"), 1000); // CLEANUP
		emit(update_title());
	}
}

int sem_model::parent_of(int i_oId)
{
        for (int i=0; i<m_oLinks.size(); i++)
        {
                QPoint l_oP = m_oLinks.at(i);
                if (l_oP.y() == i_oId)
			return l_oP.x();
        }
	return NO_ITEM;
}

int sem_model::num_children(int i_iParent)
{
	int l_iCnt = 0;
	for (int i=0; i<m_oLinks.size(); i++)
	{
		QPoint l_oP = m_oLinks.at(i);
		if (l_oP.x() == i_iParent) l_iCnt++;
	}
	//qDebug()<<"num children for "<<i_iParent<<" "<<l_iCnt;
	return l_iCnt;
}

void sem_model::generate_docs(const QString &i_oFile, const QString &i_sDirName, const QString &i_sLocation)
{
	int l_iRoot = choose_root();
	if (l_iRoot == NO_ITEM)
	{
		//qDebug()<<"root is -1";
		return;
	}

	QFile l_o2(i_oFile);
	if (!l_o2.open(QIODevice::ReadOnly))
	{
		emit sig_message(trUtf8("Code generation failed: missing file %1").arg(i_oFile), 5000);
		return;
	}

	QByteArray l_oBa = l_o2.readAll();
	l_o2.close();

	m_sOutDir = i_sLocation;

	bind_node::_root = create_tree(l_iRoot);
	bind_node::set_var(notr("temp_dir"), m_sTempDir);
	bind_node::set_var(notr("outdir"), i_sLocation);
	bind_node::set_var(notr("pname"), i_sDirName);
	bind_node::set_var(notr("fulldoc"), doc_to_xml());
	bind_node::set_var(notr("hints"), m_sHints);
	bind_node::set_var(notr("namet"), i_oFile);
	bind_node::set_var(notr("preview"), ""); // leave this right here! get the file generated

	QList<int> lst;
	mem_sel *sel = new mem_sel(this);
	sel->sel = lst;
	sel->apply();

	foreach (int l_iVal, m_oItems.keys())
	{
		data_item *l_oData = m_oItems.value(l_iVal);
		// the diagram view is the only one for now
		if (l_oData->m_iDataType != VIEW_DIAG)
			continue;
		notify_export_item(l_oData->m_iId);
	}

	if (!init_py())
	{
		emit sig_message(trUtf8("Missing bindings for opening files"), 5000);
		return;
	}
	PyRun_SimpleString(l_oBa.constData());

	delete bind_node::_root;
	bind_node::_root = NULL;

	emit sig_preview();
}

bind_node* sem_model::create_tree(int i_i)
{
	Q_ASSERT(i_i!=0);
	bind_node * l_oNode = new bind_node();
	l_oNode->m_oItem = m_oItems[i_i];
	//l_oNode->_id = i_i;
	//fill_from_node(l_oNode);

        for (int i=0; i<m_oLinks.size(); i++)
        {
                QPoint l_oP = m_oLinks.at(i);
                if (l_oP.x() != i_i) continue;

		bind_node *l_oNew = create_tree(l_oP.y());
		l_oNode->_children.push_back(l_oNew);
        }

	return l_oNode;
}

int sem_model::choose_root()
{
	int l_oCand = NO_ITEM;
	int l_oCandSize = 0;
	for (int i=0; i<m_oLinks.size(); i++)
        {
                QPoint l_oP = m_oLinks.at(i);

		if (parent_of(l_oP.x()) > NO_ITEM) continue;
		if (l_oP.x() == l_oCand) continue;

		int l_oNewSize = size_of(l_oP.x());
		if (l_oNewSize > l_oCandSize)
		{
			l_oCand = l_oP.x();
			l_oCandSize = l_oNewSize;
		}
	}

	if (l_oCandSize == 0) return NO_ITEM;
	return l_oCand;
}

int sem_model::size_of(int i_i)
{
	// warning, recursive
	int l_i = 0;
	for (int i=0; i<m_oLinks.size(); i++)
	{
		QPoint l_oP = m_oLinks.at(i);
		if (l_oP.x() == i_i)
			l_i += size_of(l_oP.y());
	}
	return ++l_i;
}

QIcon fetch_icon(const QString& i_sName, int i_iSize)
{
	QString l_sStyle = notr("crystalsvg");
	QString l_sSize = QString::number(i_iSize);
	QStringList l_o = QString(ICONS).split(':');
	foreach (QString l_s, l_o)
	{
		QString l_sFile = l_s+"/"+l_sStyle+"/"+l_sSize+"x"+l_sSize+"/actions/"+i_sName+".png";
		if (!QFileInfo(l_sFile).isReadable()) continue;

		QIcon l_oIcon = QIcon(l_sFile);
		if (!l_oIcon.isNull()) return l_oIcon;
	}

	return QIcon();
	//return QIcon(notr(":/images/punto.png"));
}

html_converter::html_converter() : QXmlDefaultHandler() {}

bool html_converter::startElement(const QString&, const QString&, const QString& i_sName, const QXmlAttributes& i_oAttrs)
{
	if (i_sName == notr("li"))
	{
		m_oTotale<<notr("<li>");
	}

	m_sBuf.clear();
	return true;
}

sem_model::sem_model(QObject* i_oParent) : QObject(i_oParent)
{
	num_seq = 1;
	num_seq = 1;
	m_sOutDir = "";
	m_iTimerValue = 21 / 4;
	m_bDirty = false;

	m_oTimer = NULL;
	m_sOutProject = "";
	m_sOutTemplate = "";
	init_temp_dir();
}

bool html_converter::endElement(const QString&, const QString&, const QString& i_sName)
{
	if (i_sName == notr("p"))
	{
		m_oTotale<<notr("<p>")<<m_sBuf<<notr("</p>");
	}
	else if (i_sName == notr("li"))
	{
		m_oTotale<<m_sBuf<<notr("</li>");
	}
	return true;
}

bool html_converter::characters(const QString &i_s)
{
        m_sBuf += i_s;
        return true;
}

data_item* sem_model::operator+(const int y)
{
	return m_oItems.value(y);
}

void sem_model::slot_undo() {
	if (!m_oUndoStack.isEmpty()) {
		mem_command *t = m_oUndoStack.pop();
		t->undo();
		m_oRedoStack.push(t);
	}
	check_undo(true);
}

void sem_model::slot_redo() {
	if (!m_oRedoStack.isEmpty()) {
		mem_command *t = m_oRedoStack.pop();
		t->redo();
		m_oUndoStack.push(t);
	}
	check_undo(true);
}

void sem_model::private_select_item(int id) {
	mem_sel *sel = new mem_sel(this);
	sel->sel.append(id);
	sel->apply();
}

void sem_model::notify_add_item(int id) {
	emit sig_add_item(id);
}

void sem_model::notify_delete_item(int id) {
	emit sig_delete_item(id);
}

void sem_model::notify_link_items(int id1, int id2) {
	emit sig_link_items(id1, id2);
}

void sem_model::notify_unlink_items(int id1, int id2) {
	emit sig_unlink_items(id1, id2);
}

void sem_model::notify_select(const QList<int>& unsel, const QList<int>& sel) {
	emit sig_select(unsel, sel);
}

void sem_model::notify_move(const QList<int>&sel, const QList<QPointF>&pos) {
	emit sig_move(sel, pos);
}

void sem_model::notify_repaint(int id) {
	emit sig_repaint(id);
}

void sem_model::notify_edit(int id) {
	emit sig_edit(id);
}

void sem_model::notify_flag(int id) {
	emit sig_flag(id);
}

void sem_model::notify_datatype(int id) {
	emit sig_datatype(id);
}

void sem_model::notify_text(int id) {
	emit sig_text(id);
}

void sem_model::notify_vars(int id) {
	emit sig_vars(id);
}

void sem_model::notify_pic(int id) {
	emit sig_pic(id);
}

void sem_model::notify_table(int id) {
	emit sig_table(id);
}

void sem_model::notify_sort(int id) {
	emit sig_sort(id);
}

void sem_model::notify_change_data(int id) {
	emit sig_change_data(id);
}

void sem_model::notify_export_item(int id) {
	emit sig_export_item(id);
}

void sem_model::notify_add_box(int id, int box) {
	emit sig_add_box(id, box);
}

void sem_model::notify_del_box(int id, int box) {
	emit sig_del_box(id, box);
}

void sem_model::notify_edit_box(int id, int box) {
	emit sig_edit_box(id, box);
}

void sem_model::notify_link_box(int id, data_link*link) {
	emit sig_link_box(id, link);
}

void sem_model::notify_unlink_box(int id, data_link*link) {
	emit sig_unlink_box(id, link);
}

void sem_model::notify_message(const QString& msg, int duration) {
	emit sig_message(msg, duration);
}

#include "sem_model.moc"

