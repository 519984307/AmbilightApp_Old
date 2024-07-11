#pragma once

#ifndef PCH_ENABLED
	#include <QVector>

	#include <utils/MemoryBuffer.h>
	#include <utils/Logger.h>
	#include <utils/settings.h>
	#include <utils/Image.h>
	#include <utils/Components.h>
#endif

class BonjourServiceRegister;
class QTcpServer;
class QLocalServer;
class FlatBufferClient;
class NetOrigin;

#define AMBILIGHTAPP_DOMAIN_SERVER QStringLiteral("ambilightapp-domain")
#define BASEAPI_FLATBUFFER_USER_LUT_FILE QStringLiteral("BASEAPI_user_lut_file")

class FlatBufferServer : public QObject
{
	Q_OBJECT
public:
	FlatBufferServer(std::shared_ptr<NetOrigin> netOrigin, const QJsonDocument& config, const QString& configurationPath, QObject* parent = nullptr);
	~FlatBufferServer() override;

signals:
	void SignalSetNewComponentStateToAllInstances(ambilightapp::Components component, bool enable);
	void SignalImportFromProto(int priority, int duration, const Image<ColorRgb>& image, QString clientDescription);

public slots:
	void handleSettingsUpdate(settings::type type, const QJsonDocument& config);
	void initServer();
	int getHdrToneMappingEnabled();
	void handlerImportFromProto(int priority, int duration, const Image<ColorRgb>& image, QString clientDescription);
	void handlerImageReceived(int priority, const Image<ColorRgb>& image, int timeout_ms, ambilightapp::Components origin, QString clientDescription);
	void signalRequestSourceHandler(ambilightapp::Components component, int instanceIndex, bool listen);

private slots:
	void handlerNewConnection();
	void handlerClientDisconnected(FlatBufferClient* client);

private:
	void startServer();
	void stopServer();
	QString GetSharedLut();
	void loadLutFile();
	void setupClient(FlatBufferClient* client);

	QTcpServer*		_server;
	QLocalServer*	_domain;
	std::shared_ptr<NetOrigin> _netOrigin;
	Logger*			_log;
	int				_timeout;
	quint16			_port;

	const QJsonDocument		_config;
	BonjourServiceRegister* _serviceRegister = nullptr;
	QVector<FlatBufferClient*> _openConnections;

	int			_hdrToneMappingMode;
	int			_realHdrToneMappingMode;	
	bool		_lutBufferInit;
	QString		_configurationPath;
	QString		_userLutFile;

	MemoryBuffer<uint8_t> _lut;
};
