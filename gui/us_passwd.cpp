//! \file us_passwd.cpp
#include "us_passwd.h"
#include "us_settings.h"
#include "us_global.h"

QString US_Passwd::getPasswd( void  )
{
  // If the pw is in global memory, return it
  QString pw = g.passwd();
  if ( ! pw.isEmpty() ) return pw;

  // See if the master pasword has been set
  QByteArray currentHash = US_Settings::UltraScanPW();

  if ( currentHash.isEmpty() )
  {
    QMessageBox::information( this,
          tr( "Password Error" ),
          tr( "The Master Password has not been set." ) );

    return QString();
  }

  // Ask the user to input the password
  while ( true ) 
  {
    bool ok;
    pw = QInputDialog::getText( 
         this, 
         tr( "Master Password" ),
         tr( "Please input your Master Password" ),
         QLineEdit::Password, 
         QString(), 
         &ok );

    // If the user cancelled, return a null string
    if ( ! ok ) return QString();
    
    // Check the hash
    QByteArray hash =
          QCryptographicHash::hash( pw.toLatin1(), QCryptographicHash::Sha1 );

    if ( hash == currentHash ) break;
    
    QMessageBox::information( this,
          tr( "Password Error" ),
          tr( "The password is incorrect." ) );
  }

  // Save the password in global memory and return
  g.setPasswd( pw );
  return pw;
}


QMap < QString, QString >  US_Passwd::getPasswd_auditTrail( QString title, QString i_text, QString user_s )
{
  // // If the pw is in global memory, return it
  // QString pw = g.passwd();
  // if ( ! pw.isEmpty() ) return pw;

  QMap < QString, QString > form_map;
  QString pw;

  // See if the master pasword has been set
  QByteArray currentHash = US_Settings::UltraScanPW();

  if ( currentHash.isEmpty() )
  {
    QMessageBox::information( this,
          tr( "Password Error" ),
          tr( "The Master Password has not been set." ) );

    return form_map;
  }

  bool reject_chosen = false;
  
  // Ask the user to input the password
  while ( true ) 
  {
    QDialog dialog(this);
    dialog.setWindowTitle( title );
    QFont font_d  = dialog.property("font").value<QFont>();
    QFontMetrics fm(font_d);
    int pixelsWide = fm.width( dialog.windowTitle() );
    qDebug() << "pixelsWide -- " << pixelsWide;
    dialog.setMinimumWidth( pixelsWide*3 );
    QFormLayout form(&dialog);
        
    // Add some text above the fields
    form.addRow(new QLabel( i_text ));
    
    // Add the lineEdits with their respective labels
    QList<QLineEdit *> fields;
    QStringList form_labels = { "User:", "Comment:", "Master Password:" }; 
    for(int i = 0; i < form_labels.size(); ++i)
      {
	//QLineEdit *lineEdit = new QLineEdit(&dialog);
	QLineEdit *lineEdit = us_lineedit();
	//QString label = form_labels[i];

	QLabel* label = us_label( form_labels[i] );

	lineEdit->setObjectName( form_labels[i] );

	if ( form_labels[i] == "Master Password:" )
	  lineEdit->setEchoMode(QLineEdit::Password);
	
	
	if ( form_labels[i] == "User:" )
	  {
	    lineEdit->setText( user_s );
	    us_setReadOnly( lineEdit, true );
	  }
	
	form.addRow( label, lineEdit);
	
	fields << lineEdit;
      }
    
    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
			       Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    
    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted)
      {
	// If the user didn't dismiss the dialog, do something with the fields
	foreach(QLineEdit * lineEdit, fields)
	  {
	    if ( lineEdit->objectName() == "Master Password:" )
	      {
		pw = lineEdit->text();
	      }
	    qDebug() << lineEdit->text();

	    form_map[ lineEdit->objectName() ] = lineEdit->text();
	  }
      }
    else 
      {
	form_map. clear();
	reject_chosen = true;
      }

    //break if Cancelled
    if ( reject_chosen )
      break;
    
    // Check the hash
    QByteArray hash =
          QCryptographicHash::hash( pw.toLatin1(), QCryptographicHash::Sha1 );

    if ( hash == currentHash && !form_map["Comment:"].isEmpty()  )
      break;
    
    QMessageBox::critical( this,
          tr( "No Comment and/or Password Error" ),
          tr( "<font color='red'><b>ERROR:</b></font> The Comment field is empty, or the password is incorrect." ) );
  }

  // // Save the password in global memory and return
  // g.setPasswd( pw );
  
  return form_map;
}
