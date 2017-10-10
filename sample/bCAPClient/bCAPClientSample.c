#include <stdio.h>

#include "stdint.h"
#include "bCAPClient/bcap_client.h"

int main(void)
{
	int fd;
	uint32_t hCtrl, hVarP;
	HRESULT hr;

	/* Open connection */
	// Change the first argument to what you want.
	// <TCP> "tcp:192.168.0.1:5007"
	// <UDP> "udp:192.168.0.1:5007"
	// <COM> "com:1:38400"
	hr = bCap_Open_Client("tcp:192.168.0.1", 1000, 0, &fd);
	if(SUCCEEDED(hr)){
		float *pData;
		BSTR bstrName, bstrProv, bstrMachine, bstrOpt;
		VARIANT vntP;

		/* Service start */
		bCap_ServiceStart(fd, NULL);

		bstrName = SysAllocString(L"");
		bstrProv = SysAllocString(L"CaoProv.DENSO.VRC");
		bstrMachine = SysAllocString(L"192.168.0.1");
		bstrOpt = SysAllocString(L"");

		/* Connect controller */
		hr = bCap_ControllerConnect(fd, bstrName, bstrProv, bstrMachine, bstrOpt, &hCtrl);

		SysFreeString(bstrName);
		SysFreeString(bstrProv);
		SysFreeString(bstrMachine);
		SysFreeString(bstrOpt);

		if(SUCCEEDED(hr)){
			bstrName = SysAllocString(L"P1");
			bstrOpt = SysAllocString(L"");

			/* Get variable handle */
			hr = bCap_ControllerGetVariable(fd, hCtrl, bstrName, bstrOpt, &hVarP);

			SysFreeString(bstrName);
			SysFreeString(bstrOpt);

			if(SUCCEEDED(hr)){
				VariantInit(&vntP);

				/* Read P1 value */
				hr = bCap_VariableGetValue(fd, hVarP, &vntP);
				if(SUCCEEDED(hr)){
					SafeArrayAccessData(vntP.parray, (void **)&pData);

					printf("P1: %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %d\n",
						pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], (int)pData[6]);

					pData[0] += 1; // X
					pData[1] += 2; // Y
					pData[2] += 3; // Z
					pData[3] += 4; // RX
					pData[4] += 5; // RY
					pData[5] += 6; // RZ
					pData[6] = -1; // Fig

					SafeArrayUnaccessData(vntP.parray);

					/* Write new P1 value */
					hr = bCap_VariablePutValue(fd, hVarP, vntP);
				}

				VariantClear(&vntP);

				/* Release variable handle */
				bCap_VariableRelease(fd, &hVarP);
			}

			/* Disconnect controller */
			bCap_ControllerDisconnect(fd, &hCtrl);
		}

		/* Service stop */
		bCap_ServiceStop(fd);

		/* Close connection */
		bCap_Close_Client(&fd);
	}

	return 0;
};
