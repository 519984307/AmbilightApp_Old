#pragma once

// Ambilight App includes
#include "ProviderSpi.h"

class LedDeviceSk6822SPI : public ProviderSpi
{
public:
	explicit LedDeviceSk6822SPI(const QJsonObject& deviceConfig);
	static LedDevice* construct(const QJsonObject& deviceConfig);

private:
	bool init(const QJsonObject& deviceConfig) override;
	int write(const std::vector<ColorRgb>& ledValues) override;

	const int SPI_BYTES_PER_COLOUR;
	const int SPI_BYTES_WAIT_TIME;
	const int SPI_FRAME_END_LATCH_BYTES;

	uint8_t bitpair_to_byte[4];
};
