#pragma once

#ifndef PCH_ENABLED
	#include <QDateTime>
	#include <chrono>
#endif

#include <leddevice/LedDevice.h>

class QFile;

class LedDeviceFile : public LedDevice
{
public:
	explicit LedDeviceFile(const QJsonObject& deviceConfig);
	~LedDeviceFile() override;

	static LedDevice* construct(const QJsonObject& deviceConfig);

protected:
	bool init(const QJsonObject& deviceConfig) override;
	int open() override;
	int close() override;
	int write(const std::vector<ColorRgb>& ledValues) override;	

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _lastWriteTimeNano;
	
	QFile* _file;

	QString _fileName;
	bool _printTimeStamp;
};
