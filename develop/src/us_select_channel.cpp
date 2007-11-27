#include "../include/us_select_channel.h"

US_SelectChannel::US_SelectChannel(QString *tmp_selected_channel, unsigned int **channels, QWidget *parent, const char *name) : QDialog( parent, name, false)
{
	int spacing=2;
	int xpos=spacing, ypos=spacing;
	int buttonw = 280;
	int buttonh = 26;
	USglobal = new US_Config();

	selected_channel = tmp_selected_channel;
	QString str;
	setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );

	setCaption("Channel Selection");
	
	lbl_blank = new QLabel(tr("Your fluorescence experiment\n"
									  "contains multiple channels\n"
									  "per cell\n"
	                          "\nPlease select one channel\n"
									  "to be edited:"),this);
	lbl_blank->setAlignment(AlignCenter|AlignVCenter);
	lbl_blank->setGeometry(xpos, ypos, buttonw, 5*buttonh);
	lbl_blank->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_blank->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += spacing + 6*buttonh;

	lb_channel = new QListBox(this, "software");
	lb_channel->setGeometry(xpos, ypos, buttonw, buttonh*4);
	lb_channel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));	for (int i=0; i<5; i++)
	{
		if ((*channels)[i] > 0)
		{
			switch (i)
			{
				case 0:
				{
					lb_channel->insertItem(str.sprintf("Channel A (%d scans total)", (*channels)[i]));
					break;
				}
				case 1:
				{
					lb_channel->insertItem(str.sprintf("Channel B (%d scans total)", (*channels)[i]));
					break;
				}
				case 2:
				{
					lb_channel->insertItem(str.sprintf("Channel C (%d scans total)", (*channels)[i]));
					break;
				}
				case 3:
				{
					lb_channel->insertItem(str.sprintf("Channel D (%d scans total)", (*channels)[i]));
					break;
				}
				case 4:
				{
					lb_channel->insertItem(str.sprintf("Channel E (%d scans total)", (*channels)[i]));
					break;
				}
			}
		}
	}
	lb_channel->setSelected(0, true);
	select_channel(0);
	connect(lb_channel, SIGNAL(selected(int)), SLOT(select_channel(int)));
	connect(lb_channel, SIGNAL(highlighted(int)), SLOT(select_channel(int)));
	
	ypos += spacing + 5*buttonh;

	pb_ok = new QPushButton(tr("OK"), this);
	Q_CHECK_PTR(pb_ok);
	pb_ok->setAutoDefault(false);
	pb_ok->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_ok, SIGNAL(clicked()), SLOT(accept()));

	xpos += buttonw/2+1;
	
	pb_cancel = new QPushButton(tr("Cancel"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()));
	
	ypos +=30;
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, buttonw+2*spacing, ypos);
	setMinimumSize(buttonw+2*spacing, ypos);
	
}

US_SelectChannel::~US_SelectChannel()
{
}

void US_SelectChannel::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_SelectChannel::select_channel(int val)
{
	switch (val)
	{
		case 0:
		{
			*selected_channel = "A";
			break;
		}
		case 1:
		{
			*selected_channel = "B";
			break;
		}
		case 2:
		{
			*selected_channel = "C";
			break;
		}
		case 3:
		{
			*selected_channel = "D";
			break;
		}
		case 4:
		{
			*selected_channel = "E";
			break;
		}
	}
}
