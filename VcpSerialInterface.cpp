/*
 * VcpSerialInterface.cpp
 *
 *  Created on: 2015. okt. 25.
 *      Author: peti
 */

#include "VcpSerialInterface.h"
#include "mprotocol-server/AbstractUpLayer.h"

#define LOG_PRINTF(...)

VcpSerialInterface* VcpSerialInterface::instance = NULL;

static int8_t SerialIface_CDC_Receive_FS (uint8_t* Buf, uint32_t *Len);


VcpSerialInterface::VcpSerialInterface(USBD_HandleTypeDef* usbDevice, USBD_CDC_ItfTypeDef* fops, uint16_t txBufferSize) {
	this->usbDevice = usbDevice;
	this->fops = fops;

	this->txBuffer = new uint8_t[txBufferSize];
	this->txBufferSize = txBufferSize;
	this->txPosition = 0;
	this->txOverrunCount = 0;
	instance = this;
}

VcpSerialInterface* VcpSerialInterface::getExistingInstance() {
	return instance;
}

VcpSerialInterface::~VcpSerialInterface() {
	delete[] txBuffer;
}

void VcpSerialInterface::listen() {
	  fops->Receive = SerialIface_CDC_Receive_FS;
}

bool VcpSerialInterface::isOpen() {
	return (usbDevice != NULL);
}

void VcpSerialInterface::handler() {
	// USB CDC internal TX buffer limitation
	static const uint16_t PacketSize = 256;

	if (usbDevice == NULL) {
		return;	// cannot use VCP before the USB device is connected
	}

	if (txPosition != 0) {
		// first packet start address
		uint8_t* data = txBuffer;
		while(data < &txBuffer[txPosition]) {
			// packet length is number of remaining bytes or max packet size
			uint16_t packetLen = MIN(PacketSize, &txBuffer[txPosition] - data);
			// transmit current packet (!non-blocking call!)
			CDC_Transmit_FS(data, packetLen);

			// get handle to USB CDC device
			USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*) usbDevice->pClassData;
			// wait for CDC to finish transmission
			while (hcdc->TxState == 1);

			data += packetLen;
		}
		txPosition = 0;
	}

	// create log message that will be sent out on the next call
	if (txOverrunCount > 0) {
		LOG_PRINTF("TX overrun %d", txOverrunCount);
		txOverrunCount = 0;
	}
}

bool VcpSerialInterface::writeBytes(const uint8_t* bytes, uint16_t length) {
	if (txPosition + length > txBufferSize) {
		txOverrunCount += length;
		return false;
	}
	memcpy(txBuffer + txPosition, bytes, length);
	txPosition += length;
	return true;
}

bool VcpSerialInterface::receiveBytes(const uint8_t* bytes, uint16_t len) {
	USBD_CDC_ReceivePacket(usbDevice);
	if (uplayer == NULL) {
		return false;
	}
	return (uplayer->receiveBytes(bytes, len));
}

static int8_t SerialIface_CDC_Receive_FS(uint8_t* Buf, uint32_t *Len) {
	bool success = VcpSerialInterface::getExistingInstance()->receiveBytes(Buf, (uint16_t) *Len);

	return success ? USBD_OK : USBD_BUSY;
}
