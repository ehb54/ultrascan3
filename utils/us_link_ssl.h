#ifndef LINK_H_
#define LINK_H_

#include "us_extern.h"
#include <QSslSocket>

class US_UTIL_EXTERN Link : public QObject
{
	Q_OBJECT

public:
	Link( QString );
	//Link();
	bool connectToServer(const QString&, const int);
	void disconnectFromServer( void );

	void stopOptima( void );
	void skipOptimaStage( void );

	QString certPath;
	bool    disconnected_itself;
	bool    connected_itself;
	
	QString elapsedTime;
	QString temperature;
	QString rpm;
	QString omega2T;
	QString vacuum;
	QString current_stage;
	QString tot_stages;
	QString tot_scans;
	QString running_scans;

private:
	QSslSocket server;

Q_SIGNALS:
	void disconnected(void);

private slots:
	void sslErrors(const QList<QSslError> &errors);
	void rx(void);
	void serverDisconnect(void);
};

#endif /* LINK_H_ */
