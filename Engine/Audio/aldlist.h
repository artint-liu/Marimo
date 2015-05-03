#ifndef ALDEVICELIST_H
#define ALDEVICELIST_H

#pragma warning(disable: 4786)  //disable warning "identifier was truncated to '255' characters in the browser information"


typedef struct
{
  clStringA         strDeviceName;
  int               iMajorVersion;
  int               iMinorVersion;
  unsigned int      uiSourceCount;
  clStringArrayA*   pvstrExtensions;
  bool              bSelected;
} ALDEVICEINFO, *LPALDEVICEINFO;

class ALDeviceList
{
public:
  typedef clvector<ALDEVICEINFO> DeviceInfoArray;
private:
  DeviceInfoArray vDeviceInfo;
  int defaultDeviceIndex;
  int filterIndex;

public:
  ALDeviceList                      ();
  virtual ~ALDeviceList             ();
  int       GetNumDevices           ();
  CLLPCSTR  GetDeviceName           (int index);
  void      GetDeviceVersion        (int index, int *major, int *minor);
  CLUINT    GetMaxNumSources        (int index);
  bool      IsExtensionSupported    (int index, char *szExtName);
  int       GetDefaultDevice        ();
  void      FilterDevicesMinVer     (int major, int minor);
  void      FilterDevicesMaxVer     (int major, int minor);
  void      FilterDevicesExtension  (char *szExtName);
  void      ResetFilters            ();
  int       GetFirstFilteredDevice  ();
  int       GetNextFilteredDevice   ();

private:
  unsigned int GetMaxNumSources();
};

#endif // ALDEVICELIST_H
