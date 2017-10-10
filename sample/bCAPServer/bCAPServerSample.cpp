#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _MSC_VER
#include <Windows.h>
#include <ATLComTime.h>
#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
#include "bcap_server.h"
#else
#include "bcap_core/bCAPServer/bcap_server.h"
#endif

static void
print_variant(VARIANT *vnt, int depth)
{
#ifdef _MSC_VER
  LONG i, lb, ub;
  char chTab[128];
#else
  int32_t i, lb, ub;
  char chTab[128], chTmp[128];
#endif
  void *pData;
  VARIANT vntTmp, vntDate;
  VariantInit(&vntTmp);
  VariantInit(&vntDate);

  for (i = 0; i < depth; i++) {
    chTab[i] = '\t';
  }
  chTab[i] = '\0';

  printf("%svt: %d\n", chTab, vnt->vt);
  printf("%sdata: ", chTab);

  if (vnt->vt & VT_ARRAY) {
    SafeArrayGetLBound(vnt->parray, 1, &lb);
    SafeArrayGetUBound(vnt->parray, 1, &ub);

    SafeArrayAccessData(vnt->parray, &pData);

    printf("%s", chTab);

    switch (vnt->vt & ~VT_ARRAY) {
      case VT_I2:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%d", *((int16_t *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_I4:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%d", *((int32_t *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_R4:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%f", *((float *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_R8:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%lf", *((double *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_CY:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%lld", ((CY *) pData + i)->int64);
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_DATE:
        vntDate.vt = VT_DATE;
        for (i = 0; i <= (ub - lb); i++) {
          vntDate.date = *((DATE *) pData + i);
#ifdef _MSC_VER
          VariantChangeType(&vntTmp, &vntDate, 0, VT_BSTR);
          printf("%ls", vntTmp.bstrVal);
          VariantClear(&vntTmp);
#else
          strftime(chTmp, 128, "%Y-%m-%d %H:%M:%S",
              gmtime((DATE *) pData + i));
          printf("%s", chTmp);
#endif
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_BSTR:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%ls", *((BSTR *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_ERROR:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%08X", *((int32_t *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_BOOL:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%s",
              (*((VARIANT_BOOL *) pData + i) == VARIANT_TRUE) ?
                  "true" : "false");
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_VARIANT:
        printf("\n");
        for (i = 0; i <= (ub - lb); i++) {
          print_variant(((VARIANT *) pData + i), depth + 1);
        }
        break;
      case VT_UI1:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%u", *((uint8_t *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_UI2:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%u", *((uint16_t *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
      case VT_UI4:
        for (i = 0; i <= (ub - lb); i++) {
          printf("%u", *((uint32_t *) pData + i));
          if (i != (ub - lb))
            printf(", ");
        }
        break;
    }

    printf("\n");

    SafeArrayUnaccessData(vnt->parray);
  } else {
    switch (vnt->vt) {
      case VT_I2:
        printf("%d", vnt->iVal);
        break;
      case VT_I4:
        printf("%d", vnt->lVal);
        break;
      case VT_R4:
        printf("%f", vnt->fltVal);
        break;
      case VT_R8:
        printf("%lf", vnt->dblVal);
        break;
      case VT_CY:
        printf("%lld", vnt->cyVal.int64);
        break;
      case VT_DATE:
#ifdef _MSC_VER
        VariantChangeType(&vntTmp, vnt, 0, VT_BSTR);
        printf("%ls", vntTmp.bstrVal);
        VariantClear(&vntTmp);
#else
        strftime(chTmp, 128, "%Y-%m-%d %H:%M:%S", gmtime(&vnt->date));
        printf("%s", chTmp);
#endif
        break;
      case VT_BSTR:
        printf("%ls", vnt->bstrVal);
        break;
      case VT_ERROR:
        printf("%08X", vnt->scode);
        break;
      case VT_BOOL:
        printf("%s", (vnt->boolVal == VARIANT_TRUE) ? "true" : "false");
        break;
      case VT_UI1:
        printf("%u", vnt->bVal);
        break;
      case VT_UI2:
        printf("%u", vnt->uiVal);
        break;
      case VT_UI4:
        printf("%u", vnt->ulVal);
        break;
      default:
        printf("(nil)");
        break;
    }

    printf("\n");
  }
}

static void
print_args(const char *chName, VARIANT *vntArgs, int16_t Argc)
{
  int16_t i;
  printf("%s\n", chName);
  for (i = 0; i < Argc; i++) {
    printf("Arg[%d]\n", i + 1);
    print_variant(&vntArgs[i], 1);
  }
  printf("\n");
}

static HRESULT
ServiceStart(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ServiceStart", vntArgs, Argc);
  return S_OK;
}

static HRESULT
ServiceStop(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ServiceStop", vntArgs, Argc);
  return S_OK;
}

static HRESULT
ControllerConnect(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerConnect", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 1L;

  return S_OK;
}

static HRESULT
ControllerDisconnect(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerDisconnect", vntArgs, Argc);

  return S_OK;
}

static HRESULT
ControllerGetExtension(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetExtension", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 2L;

  return S_OK;
}

static HRESULT
ControllerGetFile(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetFile", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 3L;

  return S_OK;
}

static HRESULT
ControllerGetRobot(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetRobot", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 4L;

  return S_OK;
}

static HRESULT
ControllerGetTask(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetTask", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 5L;

  return S_OK;
}

static HRESULT
ControllerGetVariable(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetVariable", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 6L;

  return S_OK;
}

static HRESULT
ControllerGetCommand(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetCommand", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 7L;

  return S_OK;
}

static HRESULT
ControllerGetExtensionNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("ControllerGetExtensionNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Controller Extension1");
  pData[1] = SysAllocString(L"Controller Extension2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
ControllerGetFileNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("ControllerGetFileNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Controller File1");
  pData[1] = SysAllocString(L"Controller File2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
ControllerGetRobotNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("ControllerGetRobotNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Controller Robot1");
  pData[1] = SysAllocString(L"Controller Robot2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
ControllerGetTaskNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("ControllerGetTaskNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Controller Task1");
  pData[1] = SysAllocString(L"Controller Task2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
ControllerGetVariableNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("ControllerGetVariableNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Controller Variable1");
  pData[1] = SysAllocString(L"Controller Variable2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
ControllerGetCommandNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("ControllerGetCommandNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Controller Command1");
  pData[1] = SysAllocString(L"Controller Command2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
ControllerExecute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerExecute", vntArgs, Argc);

  return S_OK;
}

static HRESULT
ControllerGetMessage(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetMessage", vntArgs, Argc);

  if ((rand() % 100) == 1) {
    vntRet->vt = VT_I4;
    vntRet->lVal = 8L;
    return S_OK;
  } else {
    return S_FALSE;
  }
}

static HRESULT
ControllerGetAttribute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetAttribute", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
ControllerGetHelp(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetHelp", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Controller Help");

  return S_OK;
}

static HRESULT
ControllerGetName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Controller Name");

  return S_OK;
}

static HRESULT
ControllerGetTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetTag", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
ControllerPutTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerPutTag", vntArgs, Argc);

  return S_OK;
}

static HRESULT
ControllerGetID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerGetID", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
ControllerPutID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ControllerPutID", vntArgs, Argc);

  return S_OK;
}

static HRESULT
ExtensionGetVariable(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionGetVariable", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 9L;

  return S_OK;
}

static HRESULT
ExtensionGetVariableNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("ExtensionGetVariableNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Extension Variable1");
  pData[1] = SysAllocString(L"Extension Variable2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
ExtensionExecute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionExecute", vntArgs, Argc);

  return S_OK;
}

static HRESULT
ExtensionGetAttribute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionGetAttribute", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
ExtensionGetHelp(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionGetHelp", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Extension Help");

  return S_OK;
}

static HRESULT
ExtensionGetName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionGetName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Extension Name");

  return S_OK;
}

static HRESULT
ExtensionGetTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionGetTag", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
ExtensionPutTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionPutTag", vntArgs, Argc);

  return S_OK;
}

static HRESULT
ExtensionGetID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionGetID", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0;

  return S_OK;
}

static HRESULT
ExtensionPutID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionPutID", vntArgs, Argc);

  return S_OK;
}

static HRESULT
ExtensionRelease(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("ExtensionRelease", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileGetFile(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetFile", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 10L;

  return S_OK;
}

static HRESULT
FileGetVariable(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetVariable", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 11L;

  return S_OK;
}

static HRESULT
FileGetFileNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("FileGetFileNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"File File1");
  pData[1] = SysAllocString(L"File File2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
FileGetVariableNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("FileGetVariableNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"File Variable1");
  pData[1] = SysAllocString(L"File Variable2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
FileExecute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileExecute", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileCopy(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileCopy", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileDelete(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileDelete", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileMove(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileMove", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileRun(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileRun", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"FileRun");

  return S_OK;
}

static HRESULT
FileGetDateCreated(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetDateCreated", vntArgs, Argc);

  vntRet->vt = VT_DATE;
  vntRet->date = 0;

  return S_OK;
}

static HRESULT
FileGetDateLastAccessed(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetDateLastAccessed", vntArgs, Argc);

  vntRet->vt = VT_DATE;
#ifdef _MSC_VER
  vntRet->date = COleDateTime::GetCurrentTime();
#else
  time(&vntRet->date);
#endif

  return S_OK;
}

static HRESULT
FileGetDateLastModified(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetDateLastModified", vntArgs, Argc);

  vntRet->vt = VT_DATE;
  vntRet->date = 0;

  return S_OK;
}

static HRESULT
FileGetPath(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetPath", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Path");

  return S_OK;
}

static HRESULT
FileGetSize(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetSize", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
FileGetType(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetType", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Type");

  return S_OK;
}

static HRESULT
FileGetValue(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetValue", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"FileValue");

  return S_OK;
}

static HRESULT
FilePutValue(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FilePutValue", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileGetAttribute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetAttribute", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
FileGetHelp(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetHelp", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"File Help");

  return S_OK;
}

static HRESULT
FileGetName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"File Name");

  return S_OK;
}

static HRESULT
FileGetTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetTag", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
FilePutTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FilePutTag", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileGetID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileGetID", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
FilePutID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FilePutID", vntArgs, Argc);

  return S_OK;
}

static HRESULT
FileRelease(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("FileRelease", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotGetVariable(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotGetVariable", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 12L;

  return S_OK;
}

static HRESULT
RobotGetVariableNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("RobotGetVariableNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Robot Variable1");
  pData[1] = SysAllocString(L"Robot Variable2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
RobotExecute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotExecute", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotAccelerate(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotAccelerate", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotChange(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotChange", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotChuck(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotChuck", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotDrive(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotDrive", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotGoHome(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotGoHome", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotHalt(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotHalt", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotHold(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotHold", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotMove(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotMove", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotRotate(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotRotate", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotSpeed(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotSpeed", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotUnchuck(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotUnchuck", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotUnhold(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotUnhold", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotGetAttribute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotGetAttribute", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
RobotGetHelp(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotGetHelp", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Robot Help");

  return S_OK;
}

static HRESULT
RobotGetName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotGetName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Robot Name");

  return S_OK;
}

static HRESULT
RobotGetTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotGetTag", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
RobotPutTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotPutTag", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotGetID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotGetID", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
RobotPutID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotPutID", vntArgs, Argc);

  return S_OK;
}

static HRESULT
RobotRelease(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("RobotRelease", vntArgs, Argc);

  return S_OK;
}

static HRESULT
TaskGetVariable(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskGetVariable", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 13L;

  return S_OK;
}

static HRESULT
TaskGetVariableNames(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  BSTR* pData;
  print_args("TaskGetVariableNames", vntArgs, Argc);

  vntRet->vt = VT_BSTR | VT_ARRAY;
  vntRet->parray = SafeArrayCreateVector(VT_BSTR, 0L, 2L);
  SafeArrayAccessData(vntRet->parray, (void **) &pData);
  pData[0] = SysAllocString(L"Task Variable1");
  pData[1] = SysAllocString(L"Task Variable2");
  SafeArrayUnaccessData(vntRet->parray);

  return S_OK;
}

static HRESULT
TaskExecute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskExecute", vntArgs, Argc);

  return S_OK;
}

static HRESULT
TaskStart(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskStart", vntArgs, Argc);

  return S_OK;
}

static HRESULT
TaskStop(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskStop", vntArgs, Argc);

  return S_OK;
}

static HRESULT
TaskDelete(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskDelete", vntArgs, Argc);

  return S_OK;
}

static HRESULT
TaskGetFileName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskGetFileName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Task FileName");

  return S_OK;
}

static HRESULT
TaskGetAttribute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskGetAttribute", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
TaskGetHelp(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskGetHelp", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Task Help");

  return S_OK;
}

static HRESULT
TaskGetName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskGetName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Task Name");

  return S_OK;
}

static HRESULT
TaskGetTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskGetTag", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
TaskPutTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskPutTag", vntArgs, Argc);

  return S_OK;
}

static HRESULT
TaskGetID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskGetID", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
TaskPutID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskPutID", vntArgs, Argc);

  return S_OK;
}

static HRESULT
TaskRelease(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("TaskRelease", vntArgs, Argc);

  return S_OK;
}

static HRESULT
VariableGetDateTime(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetDateTime", vntArgs, Argc);

  vntRet->vt = VT_DATE;
#ifdef _MSC_VER
  vntRet->date = COleDateTime::GetCurrentTime();
#else
  time(&vntRet->date);
#endif

  return S_OK;
}

static HRESULT
VariableGetValue(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetValue", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
VariablePutValue(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariablePutValue", vntArgs, Argc);

  return S_OK;
}

static HRESULT
VariableGetAttribute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetAttribute", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
VariableGetHelp(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetHelp", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Variable Help");

  return S_OK;
}

static HRESULT
VariableGetName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Variable Name");

  return S_OK;
}

static HRESULT
VariableGetTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetTag", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
VariablePutTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariablePutTag", vntArgs, Argc);

  return S_OK;
}

static HRESULT
VariableGetID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetID", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
VariablePutID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariablePutID", vntArgs, Argc);

  return S_OK;
}

static HRESULT
VariableGetMicrosecond(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableGetMicrosecond", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0;

  return S_OK;
}

static HRESULT
VariableRelease(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("VariableRelease", vntArgs, Argc);

  return S_OK;
}

static HRESULT
CommandExecute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandExecute", vntArgs, Argc);

  return S_OK;
}

static HRESULT
CommandCancel(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandCancel", vntArgs, Argc);

  return S_OK;
}

static HRESULT
CommandGetTimeout(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetTimeout", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
CommandPutTimeout(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandPutTimeout", vntArgs, Argc);

  return S_OK;
}

static HRESULT
CommandGetState(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetState", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
CommandGetParameters(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetParameters", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
CommandPutParameters(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandPutParameters", vntArgs, Argc);

  return S_OK;
}

static HRESULT
CommandGetResult(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetResult", vntArgs, Argc);

  vntRet->vt = VT_BOOL;
  vntRet->boolVal = VARIANT_FALSE;

  return S_OK;
}

static HRESULT
CommandGetAttribute(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetAttribute", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
CommandGetHelp(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetHelp", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Command Help");

  return S_OK;
}

static HRESULT
CommandGetName(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetName", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Command Name");

  return S_OK;
}

static HRESULT
CommandGetTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetTag", vntArgs, Argc);

  vntRet->vt = VT_EMPTY;

  return S_OK;
}

static HRESULT
CommandPutTag(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandPutTag", vntArgs, Argc);

  return S_OK;
}

static HRESULT
CommandGetID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandGetID", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
CommandPutID(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandPutID", vntArgs, Argc);

  return S_OK;
}

static HRESULT
CommandRelease(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("CommandRelease", vntArgs, Argc);

  return S_OK;
}

static HRESULT
MessageReply(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageReply", vntArgs, Argc);

  return S_OK;
}

static HRESULT
MessageClear(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageClear", vntArgs, Argc);

  return S_OK;
}

static HRESULT
MessageGetDateTime(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageGetDateTime", vntArgs, Argc);

  vntRet->vt = VT_DATE;
#ifdef _MSC_VER
  vntRet->date = COleDateTime::GetCurrentTime();
#else
  time(&vntRet->date);
#endif

  return S_OK;
}

static HRESULT
MessageGetDescription(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageGetDescription", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Description");

  return S_OK;
}

static HRESULT
MessageGetDestination(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageGetDestination", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Destination");

  return S_OK;
}

static HRESULT
MessageGetNumber(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageGetNumber", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
MessageGetSerialNumber(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageGetSerialNumber", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
MessageGetSource(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageGetSource", vntArgs, Argc);

  vntRet->vt = VT_BSTR;
  vntRet->bstrVal = SysAllocString(L"Source");

  return S_OK;
}

static HRESULT
MessageGetValue(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageGetValue", vntArgs, Argc);

  vntRet->vt = VT_I4;
  vntRet->lVal = 0L;

  return S_OK;
}

static HRESULT
MessageRelease(VARIANT *vntArgs, int16_t Argc, VARIANT *vntRet)
{
  print_args("MessageRelease", vntArgs, Argc);

  return S_OK;
}

static void
SetCallFunctions()
{
  bCap_SetCallFunc(ID_SERVICE_START, &ServiceStart);
  bCap_SetCallFunc(ID_SERVICE_STOP, &ServiceStop);
  bCap_SetCallFunc(ID_CONTROLLER_CONNECT, &ControllerConnect);
  bCap_SetCallFunc(ID_CONTROLLER_DISCONNECT, &ControllerDisconnect);
  bCap_SetCallFunc(ID_CONTROLLER_GETEXTENSION, &ControllerGetExtension);
  bCap_SetCallFunc(ID_CONTROLLER_GETFILE, &ControllerGetFile);
  bCap_SetCallFunc(ID_CONTROLLER_GETROBOT, &ControllerGetRobot);
  bCap_SetCallFunc(ID_CONTROLLER_GETTASK, &ControllerGetTask);
  bCap_SetCallFunc(ID_CONTROLLER_GETVARIABLE, &ControllerGetVariable);
  bCap_SetCallFunc(ID_CONTROLLER_GETCOMMAND, &ControllerGetCommand);
  bCap_SetCallFunc(ID_CONTROLLER_GETEXTENSIONNAMES,
      &ControllerGetExtensionNames);
  bCap_SetCallFunc(ID_CONTROLLER_GETFILENAMES, &ControllerGetFileNames);
  bCap_SetCallFunc(ID_CONTROLLER_GETROBOTNAMES, &ControllerGetRobotNames);
  bCap_SetCallFunc(ID_CONTROLLER_GETTASKNAMES, &ControllerGetTaskNames);
  bCap_SetCallFunc(ID_CONTROLLER_GETVARIABLENAMES, &ControllerGetVariableNames);
  bCap_SetCallFunc(ID_CONTROLLER_GETCOMMANDNAMES, &ControllerGetCommandNames);
  bCap_SetCallFunc(ID_CONTROLLER_EXECUTE, &ControllerExecute);
  bCap_SetCallFunc(ID_CONTROLLER_GETMESSAGE, &ControllerGetMessage);
  bCap_SetCallFunc(ID_CONTROLLER_GETATTRIBUTE, &ControllerGetAttribute);
  bCap_SetCallFunc(ID_CONTROLLER_GETHELP, &ControllerGetHelp);
  bCap_SetCallFunc(ID_CONTROLLER_GETNAME, &ControllerGetName);
  bCap_SetCallFunc(ID_CONTROLLER_GETTAG, &ControllerGetTag);
  bCap_SetCallFunc(ID_CONTROLLER_PUTTAG, &ControllerPutTag);
  bCap_SetCallFunc(ID_CONTROLLER_GETID, &ControllerGetID);
  bCap_SetCallFunc(ID_CONTROLLER_PUTID, &ControllerPutID);
  bCap_SetCallFunc(ID_EXTENSION_GETVARIABLE, &ExtensionGetVariable);
  bCap_SetCallFunc(ID_EXTENSION_GETVARIABLENAMES, &ExtensionGetVariableNames);
  bCap_SetCallFunc(ID_EXTENSION_EXECUTE, &ExtensionExecute);
  bCap_SetCallFunc(ID_EXTENSION_GETATTRIBUTE, &ExtensionGetAttribute);
  bCap_SetCallFunc(ID_EXTENSION_GETHELP, &ExtensionGetHelp);
  bCap_SetCallFunc(ID_EXTENSION_GETNAME, &ExtensionGetName);
  bCap_SetCallFunc(ID_EXTENSION_GETTAG, &ExtensionGetTag);
  bCap_SetCallFunc(ID_EXTENSION_PUTTAG, &ExtensionPutTag);
  bCap_SetCallFunc(ID_EXTENSION_GETID, &ExtensionGetID);
  bCap_SetCallFunc(ID_EXTENSION_PUTID, &ExtensionPutID);
  bCap_SetCallFunc(ID_EXTENSION_RELEASE, &ExtensionRelease);
  bCap_SetCallFunc(ID_FILE_GETFILE, &FileGetFile);
  bCap_SetCallFunc(ID_FILE_GETVARIABLE, &FileGetVariable);
  bCap_SetCallFunc(ID_FILE_GETFILENAMES, &FileGetFileNames);
  bCap_SetCallFunc(ID_FILE_GETVARIABLENAMES, &FileGetVariableNames);
  bCap_SetCallFunc(ID_FILE_EXECUTE, &FileExecute);
  bCap_SetCallFunc(ID_FILE_COPY, &FileCopy);
  bCap_SetCallFunc(ID_FILE_DELETE, &FileDelete);
  bCap_SetCallFunc(ID_FILE_MOVE, &FileMove);
  bCap_SetCallFunc(ID_FILE_RUN, &FileRun);
  bCap_SetCallFunc(ID_FILE_GETDATECREATED, &FileGetDateCreated);
  bCap_SetCallFunc(ID_FILE_GETDATELASTACCESSED, &FileGetDateLastAccessed);
  bCap_SetCallFunc(ID_FILE_GETDATELASTMODIFIED, &FileGetDateLastModified);
  bCap_SetCallFunc(ID_FILE_GETPATH, &FileGetPath);
  bCap_SetCallFunc(ID_FILE_GETSIZE, &FileGetSize);
  bCap_SetCallFunc(ID_FILE_GETTYPE, &FileGetType);
  bCap_SetCallFunc(ID_FILE_GETVALUE, &FileGetValue);
  bCap_SetCallFunc(ID_FILE_PUTVALUE, &FilePutValue);
  bCap_SetCallFunc(ID_FILE_GETATTRIBUTE, &FileGetAttribute);
  bCap_SetCallFunc(ID_FILE_GETHELP, &FileGetHelp);
  bCap_SetCallFunc(ID_FILE_GETNAME, &FileGetName);
  bCap_SetCallFunc(ID_FILE_GETTAG, &FileGetTag);
  bCap_SetCallFunc(ID_FILE_PUTTAG, &FilePutTag);
  bCap_SetCallFunc(ID_FILE_GETID, &FileGetID);
  bCap_SetCallFunc(ID_FILE_PUTID, &FilePutID);
  bCap_SetCallFunc(ID_FILE_RELEASE, &FileRelease);
  bCap_SetCallFunc(ID_ROBOT_GETVARIABLE, &RobotGetVariable);
  bCap_SetCallFunc(ID_ROBOT_GETVARIABLENAMES, &RobotGetVariableNames);
  bCap_SetCallFunc(ID_ROBOT_EXECUTE, &RobotExecute);
  bCap_SetCallFunc(ID_ROBOT_ACCELERATE, &RobotAccelerate);
  bCap_SetCallFunc(ID_ROBOT_CHANGE, &RobotChange);
  bCap_SetCallFunc(ID_ROBOT_CHUCK, &RobotChuck);
  bCap_SetCallFunc(ID_ROBOT_DRIVE, &RobotDrive);
  bCap_SetCallFunc(ID_ROBOT_GOHOME, &RobotGoHome);
  bCap_SetCallFunc(ID_ROBOT_HALT, &RobotHalt);
  bCap_SetCallFunc(ID_ROBOT_HOLD, &RobotHold);
  bCap_SetCallFunc(ID_ROBOT_MOVE, &RobotMove);
  bCap_SetCallFunc(ID_ROBOT_ROTATE, &RobotRotate);
  bCap_SetCallFunc(ID_ROBOT_SPEED, &RobotSpeed);
  bCap_SetCallFunc(ID_ROBOT_UNCHUCK, &RobotUnchuck);
  bCap_SetCallFunc(ID_ROBOT_UNHOLD, &RobotUnhold);
  bCap_SetCallFunc(ID_ROBOT_GETATTRIBUTE, &RobotGetAttribute);
  bCap_SetCallFunc(ID_ROBOT_GETHELP, &RobotGetHelp);
  bCap_SetCallFunc(ID_ROBOT_GETNAME, &RobotGetName);
  bCap_SetCallFunc(ID_ROBOT_GETTAG, &RobotGetTag);
  bCap_SetCallFunc(ID_ROBOT_PUTTAG, &RobotPutTag);
  bCap_SetCallFunc(ID_ROBOT_GETID, &RobotGetID);
  bCap_SetCallFunc(ID_ROBOT_PUTID, &RobotPutID);
  bCap_SetCallFunc(ID_ROBOT_RELEASE, &RobotRelease);
  bCap_SetCallFunc(ID_TASK_GETVARIABLE, &TaskGetVariable);
  bCap_SetCallFunc(ID_TASK_GETVARIABLENAMES, &TaskGetVariableNames);
  bCap_SetCallFunc(ID_TASK_EXECUTE, &TaskExecute);
  bCap_SetCallFunc(ID_TASK_START, &TaskStart);
  bCap_SetCallFunc(ID_TASK_STOP, &TaskStop);
  bCap_SetCallFunc(ID_TASK_DELETE, &TaskDelete);
  bCap_SetCallFunc(ID_TASK_GETFILENAME, &TaskGetFileName);
  bCap_SetCallFunc(ID_TASK_GETATTRIBUTE, &TaskGetAttribute);
  bCap_SetCallFunc(ID_TASK_GETHELP, &TaskGetHelp);
  bCap_SetCallFunc(ID_TASK_GETNAME, &TaskGetName);
  bCap_SetCallFunc(ID_TASK_GETTAG, &TaskGetTag);
  bCap_SetCallFunc(ID_TASK_PUTTAG, &TaskPutTag);
  bCap_SetCallFunc(ID_TASK_GETID, &TaskGetID);
  bCap_SetCallFunc(ID_TASK_PUTID, &TaskPutID);
  bCap_SetCallFunc(ID_TASK_RELEASE, &TaskRelease);
  bCap_SetCallFunc(ID_VARIABLE_GETDATETIME, &VariableGetDateTime);
  bCap_SetCallFunc(ID_VARIABLE_GETVALUE, &VariableGetValue);
  bCap_SetCallFunc(ID_VARIABLE_PUTVALUE, &VariablePutValue);
  bCap_SetCallFunc(ID_VARIABLE_GETATTRIBUTE, &VariableGetAttribute);
  bCap_SetCallFunc(ID_VARIABLE_GETHELP, &VariableGetHelp);
  bCap_SetCallFunc(ID_VARIABLE_GETNAME, &VariableGetName);
  bCap_SetCallFunc(ID_VARIABLE_GETTAG, &VariableGetTag);
  bCap_SetCallFunc(ID_VARIABLE_PUTTAG, &VariablePutTag);
  bCap_SetCallFunc(ID_VARIABLE_GETID, &VariableGetID);
  bCap_SetCallFunc(ID_VARIABLE_PUTID, &VariablePutID);
  bCap_SetCallFunc(ID_VARIABLE_GETMICROSECOND, &VariableGetMicrosecond);
  bCap_SetCallFunc(ID_VARIABLE_RELEASE, &VariableRelease);
  bCap_SetCallFunc(ID_COMMAND_EXECUTE, &CommandExecute);
  bCap_SetCallFunc(ID_COMMAND_CANCEL, &CommandCancel);
  bCap_SetCallFunc(ID_COMMAND_GETTIMEOUT, &CommandGetTimeout);
  bCap_SetCallFunc(ID_COMMAND_PUTTIMEOUT, &CommandPutTimeout);
  bCap_SetCallFunc(ID_COMMAND_GETSTATE, &CommandGetState);
  bCap_SetCallFunc(ID_COMMAND_GETPARAMETERS, &CommandGetParameters);
  bCap_SetCallFunc(ID_COMMAND_PUTPARAMETERS, &CommandPutParameters);
  bCap_SetCallFunc(ID_COMMAND_GETRESULT, &CommandGetResult);
  bCap_SetCallFunc(ID_COMMAND_GETATTRIBUTE, &CommandGetAttribute);
  bCap_SetCallFunc(ID_COMMAND_GETHELP, &CommandGetHelp);
  bCap_SetCallFunc(ID_COMMAND_GETNAME, &CommandGetName);
  bCap_SetCallFunc(ID_COMMAND_GETTAG, &CommandGetTag);
  bCap_SetCallFunc(ID_COMMAND_PUTTAG, &CommandPutTag);
  bCap_SetCallFunc(ID_COMMAND_GETID, &CommandGetID);
  bCap_SetCallFunc(ID_COMMAND_PUTID, &CommandPutID);
  bCap_SetCallFunc(ID_COMMAND_RELEASE, &CommandRelease);
  bCap_SetCallFunc(ID_MESSAGE_REPLY, &MessageReply);
  bCap_SetCallFunc(ID_MESSAGE_CLEAR, &MessageClear);
  bCap_SetCallFunc(ID_MESSAGE_GETDATETIME, &MessageGetDateTime);
  bCap_SetCallFunc(ID_MESSAGE_GETDESCRIPTION, &MessageGetDescription);
  bCap_SetCallFunc(ID_MESSAGE_GETDESTINATION, &MessageGetDestination);
  bCap_SetCallFunc(ID_MESSAGE_GETNUMBER, &MessageGetNumber);
  bCap_SetCallFunc(ID_MESSAGE_GETSERIALNUMBER, &MessageGetSerialNumber);
  bCap_SetCallFunc(ID_MESSAGE_GETSOURCE, &MessageGetSource);
  bCap_SetCallFunc(ID_MESSAGE_GETVALUE, &MessageGetValue);
  bCap_SetCallFunc(ID_MESSAGE_RELEASE, &MessageRelease);
}

int
main(void)
{
  int fd = 0;
  HRESULT hr;

  SetCallFunctions();

  hr = bCap_Open_Server("tcp", 1000, &fd);
  if (SUCCEEDED(hr)) {
    getchar();
    bCap_Close_Server(&fd);
  }

  return 0;
}
