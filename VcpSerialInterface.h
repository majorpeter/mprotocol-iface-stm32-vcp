/*
 * VcpSerialInterface.h
 *
 *  Created on: 2015. okt. 25.
 *      Author: peti
 */

#ifndef VCPSERIALINTERFACE_H_
#define VCPSERIALINTERFACE_H_

#include "mprotocol-server/AbstractSerialInterface.h"
#include "usbd_cdc_if.h"

class VcpSerialInterface: public AbstractSerialInterface {
private:
	USBD_HandleTypeDef* usbDevice;
	USBD_CDC_ItfTypeDef* fops;

	uint8_t *txBuffer;
	uint16_t txBufferSize;
	uint16_t txPosition;
	uint16_t txOverrunCount;

	static VcpSerialInterface* instance;
public:
	static VcpSerialInterface* getExistingInstance();

	VcpSerialInterface(USBD_HandleTypeDef* usbDevice, USBD_CDC_ItfTypeDef* fops, uint16_t txBufferSize);
	virtual ~VcpSerialInterface();

	virtual void listen();
	virtual bool isOpen();
	virtual void handler();
	virtual bool writeBytes(const uint8_t* bytes, uint16_t length);
	bool receiveBytes(const uint8_t* bytes, uint16_t len);
};

#endif /* VCPSERIALINTERFACE_H_ */
