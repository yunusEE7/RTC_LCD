/*
 * uart_send.c
 *
 *  Created on: 05-Jan-2025
 *      Author: yunus
 */

/*
 * This program is used for updating the time of a RTC connected to a MCU
 * RTC module: DS1307
 * Target MCU: STM32F407xx
 * UART module: CP2102 USB to UART bridge
 *
 * This program is implemented with the help of Documentation provided my SILICON LABS
 * for more information please visit https://www.silabs.com/developer-tools/
 * usb-to-uart-bridge-vcp-drivers?tab=documentation
 */


#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

//CP2102 COM Port Name
#define COM7 "\\\\.\\COM7"

//initialize COM Port for UART communication
void USART_COM_Init(const char *portName);

//function to write data to UART port
uint8_t USART_WriteData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten);

//function to close COM port communicatoin
void Close_COM_Port(void);

//function to convert time parameters to 7bytes packet compatible to DS1307 RTC
void Tx_Packet(struct tm *time, uint8_t *pTxBuffer);

//function to adjust time mismatch in seconds
void Adjust_time_delay(struct tm *time, int adj_sec);

HANDLE hMasterCOM;
DCB dcbMasterInitState;
DCB dcbMaster;


int main(void) {

	//initialize COM port
	USART_COM_Init(COM7);

	uint8_t Tx_Buf[7];

	struct tm* ptr;
	time_t t;
	t = time(NULL);
	ptr = localtime(&t);

	Sleep(500);
	Adjust_time_delay(ptr, 1);

	//convert time parameters to data packets
	Tx_Packet(ptr, Tx_Buf);

	DWORD Bytes_WR = 0;
	DWORD* bytesWritten = &Bytes_WR;

	//Write data
	if( !USART_WriteData(hMasterCOM, Tx_Buf, 7, bytesWritten) ) {
		printf("Error writing to serial port\n");
	}else {
		printf("Sent %ld bytes: %s\n", *bytesWritten, Tx_Buf);
	}

	//close communication
	Close_COM_Port();

	return(0);
}


void USART_COM_Init(const char *portName) {

	/* 1. Opening a COM Port */
	hMasterCOM = CreateFile( portName,
							GENERIC_WRITE,
							0,
							0,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							0);

	if (hMasterCOM == INVALID_HANDLE_VALUE) {
        printf("Error opening serial port\n");
        return;
    }


	/* 2. Preparing an Open COM Port for Data Transmission */

	/* 2.1 Purging the COM Port*/
	PurgeComm(hMasterCOM, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	/* 2.2 Saving the COM Port's Original State */
	//DCB dcbMasterInitState;
	if( !GetCommState(hMasterCOM, &dcbMasterInitState)) {
        printf("Error getting serial port state\n");
        CloseHandle(hMasterCOM);
        return;
	}

	/* 2.3 Setting up a DCB Structure to Set the New COM State */
	dcbMaster = dcbMasterInitState;
	dcbMaster.BaudRate = CBR_115200;
	dcbMaster.Parity = NOPARITY;
	dcbMaster.ByteSize = 8;
	dcbMaster.StopBits = ONESTOPBIT;
	if (!SetCommState(hMasterCOM, &dcbMaster)) {
        printf("Error getting serial port state\n");
        CloseHandle(hMasterCOM);
        return;
	}

	Sleep(100);

}

uint8_t USART_WriteData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten) {

	uint8_t success = FALSE;
	OVERLAPPED o = {0};

	o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!WriteFile(handle, (LPCVOID)data, length, dwWritten, &o)) {
		if (GetLastError() == ERROR_IO_PENDING)
			if (WaitForSingleObject(o.hEvent, INFINITE) == WAIT_OBJECT_0)
				if (GetOverlappedResult(handle, &o, dwWritten, FALSE))
					success = TRUE;
	}
	else
		success = TRUE;

	if (*dwWritten != length)
		success = FALSE;

	CloseHandle(o.hEvent);

	return success;

}

void Close_COM_Port(void) {

	SetCommState(hMasterCOM, &dcbMasterInitState);

	Sleep(100);

	CloseHandle(hMasterCOM);

	hMasterCOM = INVALID_HANDLE_VALUE;

}


void Tx_Packet(struct tm *time, uint8_t *pTxBuffer) {
	//set current date
	pTxBuffer[0] = (uint8_t)time->tm_mday;				//day
	pTxBuffer[1] = (uint8_t)( 1 + (time->tm_mon));		//month
	pTxBuffer[2] = (uint8_t)( (time->tm_year) - 100 );	//year
	pTxBuffer[3] = (uint8_t)( 1 + (time->tm_wday));		//weekday

	//set current time


	pTxBuffer[4] = (uint8_t)(time->tm_hour);			//hour
	pTxBuffer[5] = (uint8_t)time->tm_min;				//minute
	pTxBuffer[6] = (uint8_t)time->tm_sec;				//second
}


void Adjust_time_delay(struct tm *time, int adj_sec) {

	time->tm_sec += adj_sec;
	if(time->tm_sec > 59) {
		time->tm_sec%=60;
		time->tm_min++;
	}


	if(time->tm_min > 59) {
		time->tm_min%=60;
		time->tm_hour++;
	}

	if(time->tm_hour > 23) {
		time->tm_hour%=12;
		time->tm_mday++;
	}
}
